#include "views.h"
#include "responses.h"
#include "../include/io.h"
#include "session.h"
#include "global.h"
#include <stdio.h>
#include <string.h>
#include "../include/utils.h"

#include "users.h"

void home_view(SOCKET client_socket) {
	HashTable* ht = hash_table_create();

	hash_table_insert(ht, "title", "Title", strlen("Title") + 1);

	Vector* users = query_user(global.db, Query_All, NULL, NULL);

	hash_table_insert(ht, "users", users, sizeof(*users));

	render_template(client_socket, "index.html", ht);

	hash_table_delete(&ht);
	return;
}

void about_view(SOCKET client_socket) {
	render_template(client_socket, "about.html", NULL);
	return;
}

void register_view(SOCKET client_socket) {
	render_template(client_socket, "register.html", NULL);
	return;
}

void login_view(SOCKET client_socket) {
	render_template(client_socket, "login.html", NULL);
	return;
}

void profile_view(SOCKET client_socket, char* request) {
	Session* session = get_session_from_cookies(global.db, request);

	if (!session) {
		redirect(client_socket, "/login", 1, "");
		return;
	}

	printf("Welcome, Sir#%zu", session->user_id);

	render_template(client_socket, "profile.html", NULL);
	session_free_struct(session);
	return;
}

void logout_view(SOCKET client_socket, char* request) {
	Session* session = get_session_from_cookies(global.db, request);
	
	if (session) {
		session_erase(global.db, session->session_id);
		session_free_struct(session);
	}

	redirect(client_socket, "/", 1, "");
	return;
}

// Easier way to work with html
static char* include_html(File* file) {
	char* file_data = malloc(strlen(file->data) + 1);

	if (!file_data) {
		return NULL;
	}

	strcpy(file_data, file->data);

	char* begin = strstr(file_data, "{%");
	if (!begin) {
		return NULL;
	}
	begin += strlen("{%");

	char* end = strstr(begin, "%}");
	if (!end) {
		return NULL;
	}

	*end = '\0';

	char* keyword = strstr(begin, "include");

	if (!keyword) {
		return NULL;
	}

	keyword += strlen("include");

	char include_file_name[128] = { 0 };
	for (int i = 0; keyword != end; keyword++) {
		if (keyword[0] != ' ') {
			include_file_name[i++] = keyword[0];
		}
	}

	char* path = malloc(256);
	if (!path) {
		return NULL;
	}

	sprintf(path, "./src/templates/%s", include_file_name);

	File include_file;
	include_file = io_read_file(path);
	free(path);

	char* file_prefix_end = strstr(include_file.data, "{% block %}");
	if (!file_prefix_end)
		return NULL;

	char* file_suffix_begin = strstr(file_prefix_end, "{% endblock %}");
	if (!file_suffix_begin)
		return NULL;
	file_suffix_begin += strlen("{% endblock %}");

	char* file_suffix_end = strstr(file_suffix_begin, "</html>");
	if (!file_suffix_end)
		return NULL;
	file_suffix_end += strlen("</html>\r\n\r\n");

	char* block_begin = strstr(file->data, "{% block %}");
	if (!block_begin)
		return NULL;
	block_begin += strlen("{% block %}");

	char* block_end = strstr(block_begin, "{% endblock %}");
	if (!block_end)
		return NULL;

	int out_file_size = (file_prefix_end - include_file.data) + (block_end - block_begin) + (file_suffix_end - file_suffix_begin);
	char* send_data = malloc(out_file_size);
	if (!send_data)
		return NULL;

	memcpy(send_data, include_file.data, file_prefix_end - include_file.data);
	memcpy(send_data + (file_prefix_end - include_file.data), block_begin, block_end - block_begin);
	memcpy(send_data + (file_prefix_end - include_file.data) + (block_end - block_begin),
		file_suffix_begin, file_suffix_end - file_suffix_begin);

	*(send_data + out_file_size) = '\0';

	return send_data;
}

static void process_template_data(char** pp_data, HashTable* ht) {
	char* data = *pp_data;
	// process if statements
	// conditions will be formed as a single variable and will be
	// true if variable is not NULL to keep things simple
	while (strstr(data, "{% IF") != NULL) {
		const char* if_start = strstr(data, "{% IF");
		if (!if_start) break;
		const char* if_body_start = strstr(if_start, "%}");
		if (!if_body_start) break;
		if_body_start += strlen("%}");
		const char* if_end = strstr(if_start, "{% ENDIF %}");
		if (!if_end) break;
		if_end += strlen("{% ENDIF %}");

		const char* if_body_end = NULL;
		const char* else_body_start = NULL;
		const char* else_body_end = NULL;

		// get the heyword
		const int keyword_size = (if_body_start - strlen("%}") - (if_start + strlen("{% IF")));
		char* const keyword = malloc(keyword_size + 1);
		memcpy(keyword, (if_start + strlen("{% IF")), keyword_size);
		keyword[keyword_size] = 0;

		remove_spaces(keyword);

		// determine the rest of pointers based of existence of else statement
		if (strstr(if_body_start, "{% ELSE %}") && strstr(if_body_start, "{% ELSE %}") < strstr(if_body_start, "{% ENDIF %}")) {
			if_body_end = strstr(if_body_start, "{% ELSE %}");
			else_body_start = if_body_end + strlen("{% ELSE %}");
			else_body_end = strstr(else_body_start, "{% ENDIF %}");
		}
		else {
			if_body_end = strstr(if_body_start, "{% ENDIF %}");
		}

		//whole if statement body
		const int full_if_size = if_end - if_start;
		char* full_if = malloc(full_if_size + 1);
		memcpy(full_if, if_start, full_if_size);
		full_if[full_if_size] = 0;

		//choose branch based on the keyword
		if (hash_table_at(ht, keyword)) {
			const int if_body_size = if_body_end - if_body_start;
			char* if_body = malloc(if_body_size + 1);
			memcpy(if_body, if_body_start, if_body_size);
			if_body[if_body_size] = 0;

			*pp_data = str_replace(data, full_if, if_body);
			//maybe free data??
			data = *pp_data;
			free(if_body);
		}
		else {
			if (else_body_start && else_body_end) {
				const int else_body_size = else_body_end - else_body_start;
				char* else_body = malloc(else_body_size + 1);
				memcpy(else_body, else_body_start, else_body_size);
				else_body[else_body_size] = 0;

				*pp_data = str_replace(data, full_if, else_body);
				//maybe free data??
				data = *pp_data;
				free(else_body);
			}
			else {
				*pp_data = str_replace(data, full_if, " ");
				//maybe free data??
				data = *pp_data;
			}
		}

		free(keyword);
		free(full_if);
	}
	
	// proccessing for loops
	while (strstr(data, "{% FOR")) {
		const char* loop_start = strstr(data, "{% FOR");
		if (!loop_start) break;
		const char* body_start = strstr(loop_start, "%}");
		if (!body_start) break;
		body_start += strlen("%}");
		const char* body_end = strstr(body_start, "{% ENDFOR %}");
		if (!body_end) break;
		const char* loop_end = body_end + strlen("{% ENDFOR %}");

		//extract loop condition data
		const int temp_size = (body_start - strlen("%}")) - (loop_start + strlen("{% FOR"));
		char* temp = malloc(temp_size + 1);
		memcpy(temp, loop_start + strlen("{% FOR"), temp_size);
		temp[temp_size] = 0;
		char* temp_cpy = temp;
		char* const item_name = strtok_s(temp, ":", &temp);
		char* const array_name = strtok_s(temp, ":", &temp);

		remove_spaces(item_name);
		remove_spaces(array_name);

		//get the full for loop body before proccessing
		const int before_loop_size = loop_end - loop_start;
		char* before_loop = malloc(before_loop_size + 1);
		memcpy(before_loop, loop_start, before_loop_size);
		before_loop[before_loop_size] = 0;

		Vector* v = hash_table_at(ht, array_name);
		if (!v) {
			*pp_data = str_replace(data, before_loop, " ");
			//maybe free data??
			data = *pp_data;
			continue;
		}

		free(temp_cpy);

		const int body_size = body_end - body_start;
		char* body = malloc(body_size + 1);

		memcpy(body, body_start, body_size);
		body[body_size] = 0;

		const int new_body_size = body_size * vector_size(v);
		char* new_body = malloc(new_body_size + 1);
		for (int i = 0; i < vector_size(v); i++) {
			char* temp = new_body + body_size * i;
			memcpy(temp, body, body_size);
			//replace vars with values
			//...
		}
		new_body[new_body_size] = 0;

		*pp_data = str_replace(data, before_loop, new_body);
		//maybe free data??
		data = *pp_data;

		free(body);
		free(new_body);
		free(before_loop);
	}

}

// end

void render_template(SOCKET _client, const char* filename, HashTable* ht) {
	char folder_path[] = "./src/templates/";

	char path[1024];

	sprintf(path, "%s%s", folder_path, filename);

	File file = io_read_file(path);
	if (!file.is_valid) {
		return;
	}

	char* senddata = include_html(&file);

	if (ht != NULL) {
		process_template_data(&senddata, ht);
	}

	send(_client, OK_RESPONSE_HTML, strlen(OK_RESPONSE_HTML), 0);
	send(_client, senddata, strlen(senddata), 0);
	
	//free(senddata);
}

void redirect(SOCKET _client, const char* location, int flag, char* session_id) {
	char response[1024];
	if (flag) {
		sprintf(response, "HTTP/1.1 302 Found\r\n"
			"Set-Cookie: session_id=%s; Max-Age=%d\r\n"
			"Location: %s\r\n\r\n", session_id, SESSION_EXPIRE_TIME, location);
	}
	else {
		sprintf(response, "HTTP/1.1 302 Found\r\n"
			"Location: %s\r\n\r\n", location);
	}
	send(_client, response, 1024, 0);
}
