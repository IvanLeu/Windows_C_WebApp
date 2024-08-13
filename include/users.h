#pragma once

#include <stdbool.h>
#include "sqlite3\sqlite3.h"
#include "vector.h"
#include "field_metadata.h"

typedef enum Query_Type {
	Query_None = 0,
	Query_By_ID = 1,
	Query_By_Name = 2,
	Query_By_Email = 3,
	Query_By_Name_Password = 4,
	Query_All = 5,
} Query_Type;

typedef struct User {
	field_metadata* metadata;
	size_t id;
	char* name;
	char* email;
	char* password;
} User;

User* create_user();
void delete_user(User* user);

void create_users_table(sqlite3* db);
void insert_user(sqlite3* db, const char* name, const char* email, const char* password);
Vector* query_user(sqlite3* db, Query_Type type, const char* key, const char* key1);
bool hash_password(const char* password, char* out);