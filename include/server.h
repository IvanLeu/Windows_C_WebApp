#pragma once
#include <WinSock2.h>

#define MAX_BUFFER_SIZE 1024

typedef void (*Post_Handler)(SOCKET socket, const char* data);

typedef struct Server {
	int port;
	SOCKET socket;
	WSADATA wsadata;
	struct sockaddr_in sockaddr;
} Server;

typedef struct Client {
	SOCKET client;
	size_t user_id;
} Client;

Server* server_init(int port);
void server_listen(Server* server);
void server_shutdown(Server* server);

unsigned __stdcall client_handler(SOCKET _client);
void handle_get_request(SOCKET _client, char request[MAX_BUFFER_SIZE]);
void handle_post_request(SOCKET _client, char request[MAX_BUFFER_SIZE]);
void process_form_data(SOCKET _client, char request[MAX_BUFFER_SIZE], Post_Handler handler);