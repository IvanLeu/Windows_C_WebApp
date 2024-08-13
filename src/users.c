#include "users.h"
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <wincrypt.h>

#define PASSWORD_BUFF_SIZE 256

field_metadata user_metadata[] = {
   FIELD_METADATA(User, id, "size_t"),
   FIELD_METADATA(User, name, "string"),
   FIELD_METADATA(User, email, "string"),
   FIELD_METADATA(User, password, "string"),
   METADATA_TERMINATOR //terminator
};

User* create_user()
{
	User* user = malloc(sizeof(User));
	user->name = malloc(256);
	user->email = malloc(256);
	user->password = malloc(256);
	user->metadata = user_metadata;
	return user;
}

void delete_user(User* user)
{
	if (!user) {
		return;
	}

	free(user->name);
	free(user->email);
	free(user->password);
	free(user);
	return;
}

void create_users_table(sqlite3* db)
{
	char* sql = "CREATE TABLE IF NOT EXISTS users ("
				"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
				"name TEXT NOT NULL,"
				"email TEXT NOT NULL UNIQUE,"
				"password TEXT NOT NULL);";
	char* err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}
	return;
}

void insert_user(sqlite3* db, const char* name, const char* email, const char* password)
{
	char h_password[PASSWORD_BUFF_SIZE];
	if (!hash_password(password, h_password)) {
		fprintf(stderr, "Hashing failed. Line: %d", __LINE__);
		return;
	}

	char sql[256];
	sprintf(sql, "INSERT INTO users(name, email, password) VALUES ('%s', '%s', '%s');", name, email, h_password);
	char* err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}
	return;
}

Vector* query_user(sqlite3* db, Query_Type type, const char* key, const char* key1) {
	char sql[256];

	char hashed_password[256];
	if (key1) {
		hash_password(key1, hashed_password);
	}

	switch (type) {
	case Query_By_ID:
		sprintf(sql, "SELECT * FROM users WHERE ID = %s;", key);
		break;
	case Query_By_Name:
		sprintf(sql, "SELECT * FROM users WHERE name = '%s';", key);
		break;
	case Query_By_Name_Password:
		sprintf(sql, "SELECT * FROM users WHERE name = '%s' AND password = '%s';", key, hashed_password);
		break;
	case Query_By_Email:
		sprintf(sql, "SELECT * FROM users WHERE email = '%s';", key);
		break;
	case Query_All:
		sprintf(sql, "SELECT * FROM users;");
		break;
	}

	sqlite3_stmt* stmt;
	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "Failed to fetch data: %s", sqlite3_errmsg(db));
		return NULL;
	}
	
	Vector* v = vector_create(sizeof(User), 16);

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		User* user = create_user();
		user->id = sqlite3_column_int(stmt, 0);
		strcpy(user->name, sqlite3_column_text(stmt, 1));
		strcpy(user->email, sqlite3_column_text(stmt, 2));
		strcpy(user->password, sqlite3_column_text(stmt, 3));

		vector_push_back(v, user);
	}

	sqlite3_finalize(stmt);

	return v;
}

bool hash_password(const char* password, char* hash) {
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE pbHash[PASSWORD_BUFF_SIZE];
	DWORD dwDataLen = strlen(password);
	DWORD dwHashLen = PASSWORD_BUFF_SIZE;

	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
		return false;
	}

	if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
		CryptReleaseContext(hProv, 0);
		return false;
	}

	if (!CryptHashData(hHash, (BYTE*)password, dwDataLen, 0)) {
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		return false;
	}

	if (!CryptGetHashParam(hHash, HP_HASHVAL, pbHash, &dwHashLen, 0)) {
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		return false;
	}

	for (DWORD i = 0; i < dwHashLen; i++) {
		sprintf(&hash[i * 2], "%02x", pbHash[i]);
	}

	CryptReleaseContext(hProv, 0);
	CryptDestroyHash(hHash);

	return true;
}