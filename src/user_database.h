#pragma once
#include "io.h"

#define PASSWORD_BUFF_SIZE 1024

typedef struct User {
	size_t id;
	char name[128];
	char email[128];
	char password_hash[PASSWORD_BUFF_SIZE];
} User;

typedef struct UserDatabase {
	File database;
	size_t rows;
} UserDatabase;

UserDatabase user_database_init(const char* path);
size_t user_database_add_user(UserDatabase* database, const char* name, const char* email, const char* password);
User user_database_get_user(UserDatabase* database, size_t id);
bool hash_password(const char* password, char* hash);