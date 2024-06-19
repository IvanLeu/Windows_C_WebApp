#include "user_database.h"
#include "global.h"
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <wincrypt.h>

UserDatabase user_database_init(const char* path) {
	UserDatabase database;

	File file = io_read_file(path);
	if (!file.is_valid) {
		io_file_write(NULL, 0, path);
	}

	database.database = file;
	database.rows = 0;

	return database;
}

size_t user_database_add_user(UserDatabase* database, const char* name, const char* email, const char* password) {
	char hashed_password[PASSWORD_BUFF_SIZE];
	if (!hash_password(password, hashed_password)) {
		printf("Failed to hash password");
		return -1;
	}

	char* data = malloc(strlen(name) + strlen(email) + strlen(hashed_password) + 6); // + 4 for ; after id,name, etc. + 1 for \n,+ 1 for null terminator

	database->rows = io_file_count_lines(database->database.path);

	if (!data) {
		printf("Failed to add user to database");
		return -1;
	}

	if (global.last_id == (size_t)-1)
		global.last_id = database->rows;
	else
		global.last_id += 1;

	int pos = 0;
	char id[16];
	sprintf(id, "%zu", global.last_id);
	
	memcpy(data, id, strlen(id));
	pos += strlen(id) + 1;
	data[pos - 1] = ';';
	memcpy(data + pos, name, strlen(name));
	pos += strlen(name) + 1;
	data[pos - 1] = ';';
	memcpy(data + pos, email, strlen(email));
	pos += strlen(email) + 1;
	data[pos - 1] = ';';
	memcpy(data + pos, hashed_password, strlen(hashed_password));
	pos += strlen(hashed_password);

	data[pos++] = '\n';

	io_file_append(data, pos, database->database.path);
	free(data);

	return global.last_id;
}

static void set_user_data(const char* data, User* user) {
	char* token = strtok(data, ";");
	token = strtok(NULL, ";");
	strcpy(user->name, token);
	token = strtok(NULL, ";");
	strcpy(user->email, token);
	token = strtok(NULL, ";");
	strcpy(user->password_hash, token);
}

User user_database_get_user(UserDatabase* database, size_t id) {
	User user;
	user.id = id;
	char* data = malloc(1024);
	io_file_readline(data, id + 1, database->database.path);

	set_user_data(data, &user);

	free(data);

	return user;
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