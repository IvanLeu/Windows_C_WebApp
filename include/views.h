#pragma once
#include <WinSock2.h>
#include "hash_table.h"

void render_template(SOCKET _client, const char* filename, HashTable* ht);
void redirect(SOCKET _client, const char* location, int flag, char* session_id);

void home_view(SOCKET client_socket);
void about_view(SOCKET client_socket);
void register_view(SOCKET client_socket);
void login_view(SOCKET client_socket);
void profile_view(SOCKET client_socket, char* request);
void logout_view(SOCKET client_socket, char* request);
