#include "server.h"

#include <Windows.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <stdbool.h>
#include "responses.h"
#include "io.h"
#include "global.h"
#include "hash_table.h"
#include "users.h"
#include "utils.h"
#include "session.h"
#include "views.h"

HashTable* parse_data(const char* data) {
	if (!data)
		return NULL;

	HashTable* ht = hash_table_create();

	char* data_cpy = malloc(strlen(data) + 1);
	char* temp = data_cpy;
	if (data_cpy == NULL)
		return NULL;
	strcpy(data_cpy, data);

	char* token;
	while ((token = strtok_s(data_cpy, "&", &data_cpy))) {
		char* token_cpy = malloc(strlen(token) + 1);
		char* temp = token_cpy;
		if (token_cpy == NULL)
			return NULL;
		strcpy(token_cpy, token);

		char* key = strtok_s(token_cpy, "=", &token_cpy);
		char* val = strtok_s(token_cpy, "=", &token_cpy);
		hash_table_insert(ht, key, VAL_STRING, val, strlen(val) + 1);

		free(temp);
	}

	free(temp);

	return ht;
}

static void register_handler(SOCKET sock, const char* data) {
	HashTable* user_table = parse_data(data);

	char* email = str_replace(hash_table_at(user_table, "email")->data, "%40", "@");
	if (!email) {
		email = hash_table_at(user_table, "email")->data;
	}

	insert_user(global.db, hash_table_at(user_table, "name")->data, email, hash_table_at(user_table, "password")->data);
	
	hash_table_delete(&user_table);
}

static void login_handler(SOCKET client, const char* data) {
	HashTable* user_table = parse_data(data);

	Vector* queried = query_user(global.db, Users_Query_By_Name_Password, hash_table_at(user_table, "name")->data, hash_table_at(user_table, "password")->data);

	if (vector_empty(queried)) {
		// TODO: redirect to login and push messages that name or password is incorrect
		printf("Incorrect name or password\n");
	}

	char* session_id = malloc(64);
	generate_session_id(session_id);

	User* user = vector_at(queried, 0);
	session_insert(global.db, session_id, user->id);

	vector_destroy(&queried);
	hash_table_delete(&user_table);

	redirect(client, "/", 1, session_id);
	free(session_id);
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
		home_view(_client);
	}
	else if (curr_url(request, "/about")) {
		about_view(_client);
	}
	else if (curr_url(request, "/register")) {
		register_view(_client);
	}
	else if (curr_url(request, "/login")) {
		login_view(_client);
	}
	else if (curr_url(request, "/profile")) {
		profile_view(_client, request);
	}
	else if (curr_url(request, "/logout")) {
		logout_view(_client, request);
	}
	else {
		send(_client, NOT_FOUND_RESPONSE, strlen(NOT_FOUND_RESPONSE), 0);
	}
}

void handle_post_request(SOCKET _client, char request[MAX_BUFFER_SIZE]) {
	if (curr_url(request, "/register")) {
		process_form_data(_client, request, register_handler);
		redirect(_client, "/login", 0, 0);
	}
	if (curr_url(request, "/login")) {
		process_form_data(_client, request, login_handler);
	}
}

void process_form_data(SOCKET _client, char request[MAX_BUFFER_SIZE], Post_Handler handler) {
	char* post_body = strstr(request, "\r\n\r\n");
	if (post_body != NULL) {
		post_body += 4;
		handler(_client, post_body);
	}
}

