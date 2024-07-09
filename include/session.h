#pragma once
#include "sqlite3\sqlite3.h"
#include <WinSock2.h>
#include <time.h>

#define SESSION_ID_LENGTH 32

typedef struct session {
	char* session_id;
	size_t user_id;
	time_t creation_time;
} Session;

void create_sessions_table(sqlite3* db);
void session_insert(sqlite3* db, const char* session_id, size_t user_id);
void session_set_cookies(SOCKET client, const char* session_id);
Session* session_query(sqlite3* db, const char* session_id);
void session_erase(sqlite3* db, const char* session_id);
void generate_session_id(char* output);