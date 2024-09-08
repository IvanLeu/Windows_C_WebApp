#include "views.h"
#include "responses.h"
#include "../include/io.h"
#include "session.h"
#include "global.h"
#include <stdio.h>
#include <string.h>
#include "../include/utils.h"
#include "../include/html_parser.h"

#include "users.h"
#include "posts.h"

void home_view(SOCKET client_socket) {
	HashTable* ht = hash_table_create();

	const char* title = "Home Page";
	hash_table_insert(ht, "title", VAL_STRING, title, strlen(title) + 1);

	Vector* posts = query_post(global.db, Posts_Query_All, NULL);
	hash_table_insert(ht, "posts", VAL_ARRAY, posts, sizeof(*posts));

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
