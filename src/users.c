#include "users.h"
#include <stdlib.h>
#include <stdio.h>
#include <Windows.h>
#include <wincrypt.h>

#define PASSWORD_BUFF_SIZE 256

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

void query_user(sqlite3* db, Query_Type type, const char* key)
{
	char sql[256];

	switch (type) {
	case Query_By_ID:
		sprintf(sql, "SELECT * FROM users WHERE ID = %s;", key);
		break;
	case Query_By_Name:
		sprintf(sql, "SELECT * FROM users WHERE name = '%s';", key);
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
		return;
	}
	
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		// process row ...
	}

	sqlite3_finalize(stmt);
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