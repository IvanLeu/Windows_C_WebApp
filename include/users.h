#pragma once
#include <stdbool.h>
#include "sqlite3\sqlite3.h"

typedef enum Query_Type {
	Query_None = 0,
	Query_By_ID = 1,
	Query_By_Name = 2,
	Query_By_Email = 3,
	Query_All = 4,
} Query_Type;

void create_users_table(sqlite3* db);
void insert_user(sqlite3* db, const char* name, const char* email, const char* password);
void query_user(sqlite3* db, Query_Type type, const char* key);
bool hash_password(const char* password, char* hash);