#include "views.h"
#include "responses.h"
#include "io.h"
#include "session.h"
#include "global.h"
#include <stdio.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024

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
		session_set_cookies(client_socket, "");
		redirect(client_socket, "/login");
		return;
	}

	printf("Welcome, Sir#%zu", session->user_id);

	render_template(client_socket, "index.html");
	session_free_struct(session);
	return;
}

void logout_view(SOCKET client_socket, char* request) {
	Session* session = get_session_from_cookies(global.db, request);
	
	if (!session) {
		session_set_cookies(client_socket, "");
		redirect(client_socket, "/");
		return;
	}

	session_erase(global.db, session->session_id);
	session_set_cookies(client_socket, "");

	render_template(client_socket, "index.html");

	session_free_struct(session);
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

void redirect(SOCKET _client, const char* location)
{
	char response[MAX_BUFFER_SIZE];
	sprintf(response, TEMPORARY_REDIRECT, location);
	send(_client, response, MAX_BUFFER_SIZE, 0);
}