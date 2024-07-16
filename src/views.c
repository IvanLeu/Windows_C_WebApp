#include "views.h"
#include "responses.h"
#include "../include/io.h"
#include "session.h"
#include "global.h"
#include <stdio.h>
#include <string.h>


void home_view(SOCKET client_socket) {
	HashTable* ht = hash_table_create();
	hash_table_insert(ht, "title", "Title");

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

static void process_template_data(char* data, HashTable* ht) {

	// process if statements
	// conditions will be formed as a single variable and will be
	// true if variable is not NULL to keep things simple
	while (strstr(data, "{% IF") != NULL) {
		char* begin_statement = strstr(data, "{% IF");

		char* begin = strstr(data, "{% IF");
		begin += strlen("{% IF");
		char* end = strstr(begin, "%}");
		if (!end) {
			return;
		}

		char* end_statement = strstr(end, "{% ENDIF %}");
		if (!end_statement) {
			return;
		}
		end_statement += strlen("{% ENDIF %}");

		char* end_file = strstr(end_statement, "</html>");
		if (!end_file) {
			return;
		}
		end_file += strlen("</html>\r\n\r\n");

		char* keyword = malloc(end - begin);
		int i = 0;
		for (; begin != end; begin++) {
			if (begin[0] != ' ') {
				keyword[i++] = begin[0];
			}
		}
		keyword[i] = '\0';

		end += strlen("%}");

		char* new_data = malloc(strlen(data) + 1);
		if (hash_table_at(ht, keyword) != NULL) {
			char* end_if = strstr(end, "{% ELSE %}");
			if (!end_if) {
				end_if = end_statement;
			}

			memcpy(new_data, data, begin_statement - data);
			memcpy(new_data + (begin_statement - data), end, end_if - end);
			memcpy(new_data + (begin_statement - data) + (end_if - end), end_statement, end_file - end_statement);
			int file_size = (begin_statement - data) + (end_if - end) + (end_file - end_statement);
			new_data[file_size] = '\0';
		}
		else {
			char* else_begin = strstr(end, "{% ELSE %}");
			if (else_begin) {
				else_begin += strlen("{% ELSE %}");
				memcpy(new_data, data, begin_statement - data);
				memcpy(new_data + (begin_statement - data), else_begin, end_statement - else_begin);
				memcpy(new_data + (begin_statement - data) + (end_statement - else_begin), end_statement, end_file - end_statement);
				int file_size = (begin_statement - data) + (end_statement - else_begin) + (end_file - end_statement);
				new_data[file_size] = '\0';
			}
			else {

				memcpy(new_data, data, begin_statement - data);
				memcpy(new_data + (begin_statement - data), end_statement, end_file - end_statement);
				int file_size = (begin_statement - data) + (end_file - end_statement);
				new_data[file_size] = '\0';
			}
		}

		memcpy(data, new_data, strlen(new_data) + 1);
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
		process_template_data(senddata, ht);
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
