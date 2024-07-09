#include "session.h"
#include <stdio.h>

#define SESSION_EXPIRE_TIME 36000

void session_free_struct(Session* session) {
	if (!session) {
		return;
	}
	free(session->session_id);
	free(session);
}

void create_sessions_table(sqlite3* db) {
	char* err_msg = 0;
	
	char* sql = "CREATE TABLE IF NOT EXISTS sessions ("
		"session_id TEXT PRIMARY_KEY,"
		" user_id INTEGER,"
		" creation_time INTEGER);";

	if (sqlite3_exec(db, sql, 0, 0, &err_msg) != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}

	return;
}

void session_insert(sqlite3* db, const char* session_id, size_t user_id) {
	char sql[256];
	sprintf(sql, "INSERT INTO sessions(session_id, user_id, creation_time) VALUES ('%s', %zu, %d);", session_id, user_id, time(NULL));
	char* err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}
	return;
}

void session_set_cookies(SOCKET client, const char* session_id) {
	char response[1024];
	sprintf(response, "HTTP/1.1 200 OK\r\n"
		"Set-Cookie: session_id=%s; Max-Age=%d\r\n\r\n",
		session_id, SESSION_EXPIRE_TIME);
	send(client, response, 1024, 0);
}

Session* session_query(sqlite3* db, const char* session_id) {
	Session* session = malloc(sizeof(Session));
	session->session_id = malloc(64);

	char sql[256];
	sprintf(sql, "SELECT * FROM sessions WHERE session_id = '%s'", session_id);

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s", sqlite3_errmsg(db));
		return NULL;
	}
	
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		strcpy(session->session_id, sqlite3_column_text(stmt, 0));
		session->user_id = sqlite3_column_int64(stmt, 1);
		session->creation_time = sqlite3_column_int(stmt, 2);

		sqlite3_finalize(stmt);

		return session;
	}

	sqlite3_finalize(stmt);

	return NULL;
}

void session_erase(sqlite3* db, const char* session_id) {
	char sql[256];
	sprintf(sql, "DELETE FROM sessions WHERE session_if = '%s';", session_id);
	char* err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}
	return;
}

void generate_session_id(char* output) {
	srand(time(NULL));

	char pool[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

	for (int i = 0; i < SESSION_ID_LENGTH; i++) {
		output[i] = pool[rand() % strlen(pool)];
	}

	output[SESSION_ID_LENGTH] = '\0';

	return;
}