#include "server.h"

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <stdbool.h>
#include "responses.h"
#include "io.h"
#include "global.h"


static void register_handler(const char* data) {
	char* data_cpy = malloc(strlen(data));
	if (data_cpy == NULL)
		return;

	strcpy(data_cpy, data);

	char* name = malloc(128);
	char* email = malloc(128);
	char* password = malloc(256);

	if (!name || !email || !password) {
		printf("Failed to register user");
		return;
	}

	char* token = strtok(data_cpy, "&");
	strcpy(name, token);
	name += strlen("name=");
	token = strtok(NULL, "&");
	strcpy(email, token);
	email += strlen("email=");
	token = strtok(NULL, "&");
	strcpy(password, token);
	password += strlen("password=");

	user_database_add_user(&global.user_db, name, email, password);

	name -= strlen("name=");
	email -= strlen("email=");
	password -= strlen("password=");

	free(name);
	free(email);
	free(password);
}

static bool curr_url(char request[MAX_BUFFER_SIZE], const char* path) {
	if (strstr(request, "GET ") != '\0') {
		char substr[MAX_BUFFER_SIZE] = "GET ";
		memcpy(substr + 4, path, strlen(path));
		substr[strlen(path) + 4] = ' ';
		substr[strlen(path) + 5] = '\0';

		return (memcmp(request, substr, strlen(substr)) == 0);
	}
	else if (strstr(request, "POST ") != '\0') {
		char substr[MAX_BUFFER_SIZE] = "POST ";
		memcpy(substr + 5, path, strlen(path));
		substr[strlen(path) + 5] = ' ';
		substr[strlen(path) + 6] = '\0';

		return (memcmp(request, substr, strlen(substr)) == 0);
	}
	return false;
}

Server* server_init(int port) {
	Server* server = (Server*)malloc(sizeof(Server));

	if (server == NULL) {
		printf("Failed to allocate memory for server");
		exit(1);
	}

	server->port = port;

	if (WSAStartup(MAKEWORD(1, 1), &server->wsadata) != 0) {
		printf("Failed to initiate use of winsock dll");
		exit(1);
	}

	server->socket = socket(AF_INET, SOCK_STREAM, 0);

	server->sockaddr = (struct sockaddr_in){
		.sin_family = AF_INET,
		.sin_addr.s_addr = 0,
		.sin_port = htons(server->port)
	};

	if (bind(server->socket, &server->sockaddr, sizeof(server->sockaddr)) != 0) {
		printf("Failed to bind a name to a socket: %d", WSAGetLastError());
		exit(1);
	}

	return server;
}

void server_listen(Server* server) {
	HANDLE thread_handle;
	unsigned thread_id;

	if (listen(server->socket, 10) != 0) {
		printf("Failed to set socket in listen state: %d", WSAGetLastError());
		exit(1);
	}

	SOCKET _client;
	while (1) {

		_client = accept(server->socket, 0, 0);
		if (_client == INVALID_SOCKET) {
			printf("Failed to connect client socket: %d", WSAGetLastError());
			exit(1);
		}

		thread_handle = (HANDLE)_beginthreadex(NULL, 0, client_handler, _client, 0, &thread_id);
		if (thread_handle == NULL) {
			printf("Error creating thread for client.\n");
			closesocket(_client);
			continue;
		}
		CloseHandle(thread_handle);
	}
}

void server_shutdown(Server* server) {
	closesocket(server->socket);
	WSACleanup();
	free(server);
	server = NULL;
}

unsigned __stdcall client_handler(SOCKET _client) {
	char request[MAX_BUFFER_SIZE] = { 0 };
	int valread;

	valread = recv(_client, request, sizeof(request), 0);
	if (valread == SOCKET_ERROR) {
		printf("Recv failed. Error Code : %d", WSAGetLastError());
		closesocket(_client);
		_endthreadex(0);
		return 1;
	}

	if (strstr(request, "GET ") != '\0') {
		handle_get_request(_client, request);
	}
	else if (strstr(request, "POST ") != '\0') {
		handle_post_request(_client, request);
	}

	closesocket(_client);

	_endthreadex(0);
	return 0;
}

void handle_get_request(SOCKET _client, char request[MAX_BUFFER_SIZE]) {
	if (curr_url(request, "/")) {
		render_template(_client, "index.html");
	}
	else if (curr_url(request, "/about")) {
		render_template(_client, "about.html");
	}
	else if (curr_url(request, "/register")) {
		render_template(_client, "register.html");
	}
	else if (curr_url(request, "/login")) {
		render_template(_client, "login.html");
	}
	else {
		send(_client, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
}

void handle_post_request(SOCKET _client, char request[MAX_BUFFER_SIZE]) {
	if (curr_url(request, "/register")) {
		process_form_data(request, register_handler);
		redirect(_client, "/login");
	}
}

void process_form_data(char request[MAX_BUFFER_SIZE], Post_Handler handler) {
	char* post_body = strstr(request, "\r\n\r\n");
	if (post_body != NULL) {
		post_body += 4;
		handler(post_body);
	}
}

void render_template(SOCKET _client, const char* filename) {
	char folder_path[] = "../src/templates/";

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
