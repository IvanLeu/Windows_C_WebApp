#include "views.h"
#include "responses.h"
#include <stdio.h>
#include "io.h"

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

void profile_view(SOCKET client_socket) {


	render_template(client_socket, "index.html");
	return;
}

void logout_view(SOCKET client_socket) {
	char* session_id = malloc(64);
	

	render_template(client_socket, "index.html");
	return;
}

void render_template(SOCKET _client, const char* filename) {
	char folder_path[] = "./src/templates/";

	char path[1024];

	sprintf(path, "%s%s", folder_path, filename);

	File file = io_read_file(path);
	if (!file.is_valid) {
		return;
	}

	send(_client, OK_RESPONSE_HTML, strlen(OK_RESPONSE_HTML), 0);
	send(_client, file.data, file.len, 0);
}