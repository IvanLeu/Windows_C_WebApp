#include "views.h"
#include "responses.h"
#include "../include/io.h"
#include "session.h"
#include "global.h"
#include <stdio.h>
#include <string.h>


void home_view(SOCKET client_socket) {
	render_template(client_socket, "index.html");
	return;
}

void about_view(SOCKET client_socket) {
	render_template(client_socket, "about.html");
	return;
}

void register_view(SOCKET client_socket) {
	render_template(client_socket, "register.html");
	return;
}

void login_view(SOCKET client_socket) {
	render_template(client_socket, "login.html");
	return;
}

void profile_view(SOCKET client_socket, char* request) {
	Session* session = get_session_from_cookies(global.db, request);

	if (!session) {
		redirect(client_socket, "/login", 1, "");
		return;
	}

	printf("Welcome, Sir#%zu", session->user_id);

	render_template(client_socket, "profile.html");
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
		return;
	}

	strcpy(file_data, file->data);

	char* begin = strstr(file_data, "{%");
	if (!begin) {
		return;
	}
	begin += strlen("{%");

	char* end = strstr(begin, "%}");
	if (!end) {
		return;
	}

	*end = '\0';

	char* keyword = strstr(begin, "include");

	if (!keyword) {
		return;
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
		return;
	}

	sprintf(path, "./src/templates/%s", include_file_name);

	File include_file;
	include_file = io_read_file(path);
	free(path);

	char* file_prefix_end = strstr(include_file.data, "{% block %}");
	if (!file_prefix_end)
		return;

	char* file_suffix_begin = strstr(file_prefix_end, "{% endblock %}");
	if (!file_suffix_begin)
		return;
	file_suffix_begin += strlen("{% endblock %}");

	char* file_suffix_end = strstr(file_suffix_begin, "</html>");
	if (!file_suffix_end)
		return;
	file_suffix_end += strlen("</html>\r\n\r\n");

	char* block_begin = strstr(file->data, "{% block %}");
	if (!block_begin)
		return;
	block_begin += strlen("{% block %}");

	char* block_end = strstr(block_begin, "{% endblock %}");
	if (!block_end)
		return;

	int out_file_size = (file_prefix_end - include_file.data) + (block_end - block_begin) + (file_suffix_end - file_suffix_begin);
	char* send_data = malloc(out_file_size);
	if (!send_data)
		return;

	memcpy(send_data, include_file.data, file_prefix_end - include_file.data);
	memcpy(send_data + (file_prefix_end - include_file.data), block_begin, block_end - block_begin);
	memcpy(send_data + (file_prefix_end - include_file.data) + (block_end - block_begin),
		file_suffix_begin, file_suffix_end - file_suffix_begin);

	*(send_data + out_file_size) = '\0';

	return send_data;
}

static void process_template(File* file) {
	
}

// end

void render_template(SOCKET _client, const char* filename) {
	char folder_path[] = "./src/templates/";

	char path[1024];

	sprintf(path, "%s%s", folder_path, filename);

	File file = io_read_file(path);
	if (!file.is_valid) {
		return;
	}

	char* senddata = include_html(&file);

	send(_client, OK_RESPONSE_HTML, strlen(OK_RESPONSE_HTML), 0);
	send(_client, senddata, strlen(senddata) + 1, 0);
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
