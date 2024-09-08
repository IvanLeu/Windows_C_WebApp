#include "posts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

field_metadata post_metadata[] = {
   FIELD_METADATA(Post, id, "size_t"),
   FIELD_METADATA(Post, title, "string"),
   FIELD_METADATA(Post, content, "string"),
   FIELD_METADATA(Post, author, "User_PTR"),
   FIELD_METADATA(Post, creation_date, "string"),
   METADATA_TERMINATOR //terminator
};

static const char* months_en[] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December"
};

static const char* date_format_dmy(time_t time_in) {
	struct tm tm = *localtime(&time_in);
	char out_string[256];

	const unsigned short year = 1900 + (unsigned short)tm.tm_year;
	const unsigned short month = (unsigned short)tm.tm_mon + 1;
	const unsigned short mday = (unsigned short)tm.tm_mday;

	sprintf(out_string, "%s %hu, %hu", months_en[month], mday, year);

	return out_string;
}

Post* create_post() {
	Post* post = malloc(sizeof(Post));
	post->author = NULL;
	post->title = malloc(256);
	post->content = malloc(1024);
	post->creation_date = malloc(256);
	post->metadata = post_metadata;
	return post;
}

void delete_post(Post* post) {
	free(post->title);
	free(post->content);
	free(post->creation_date);
	free(post);

	return;
}

void create_posts_table(sqlite3* db) {
	char* sql = "CREATE TABLE IF NOT EXISTS posts ("
				"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
				"author_id INTEGER,"
				"title TEXT NOT NULL,"
				"content TEXT NOT NULL,"
				"creation_date TEXT NOT NULL);";
	char* err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}
	return;
}

void insert_post(sqlite3* db, const char* title, const char* content, User* author) {
	time_t current_time = time(NULL);
	char* creation_date = date_format_dmy(current_time);

	char sql[256];
	sprintf(sql, "INSERT INTO posts(author_id, title, content, creation_date) VALUES (%zu, '%s', '%s');", author->id, title, content, creation_date);
	char* err_msg = 0;
	int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s", err_msg);
		sqlite3_free(err_msg);
		exit(1);
	}
	return;
}

Vector* query_post(sqlite3* db, Post_Query_Type query_type, const char* key) {
	char sql[256];

	switch (query_type) {
	case Posts_Query_By_ID:
		sprintf(sql, "SELECT * FROM posts WHERE ID = %s;", key);
		break;
	case Posts_Query_By_Author_ID:
		sprintf(sql, "SELECT * FROM posts WHERE author_id = '%s';", key);
		break;
	case Posts_Query_By_Title:
		sprintf(sql, "SELECT * FROM posts WHERE title = '%s';", key);
		break;
	case Posts_Query_By_Date:
		sprintf(sql, "SELECT * FROM posts WHERE creation_date = '%s';", key);
		break;
	case Posts_Query_All:
		sprintf(sql, "SELECT * FROM posts;");
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
		Post* post = create_post();
		post->id = sqlite3_column_int(stmt, 0);
		
		//extract user
		Vector* users = query_user(db, Users_Query_By_ID, sqlite3_column_text(stmt, 1), NULL);
		User* author = vector_at(users, 0);
		memcpy(post->author, author, sizeof(User));
		vector_destroy(&users);

		strcpy(post->title, sqlite3_column_text(stmt, 2));
		strcpy(post->content, sqlite3_column_text(stmt, 3));
		strcpy(post->creation_date, sqlite3_column_text(stmt, 4));

		vector_push_back(v, post);
	}

	sqlite3_finalize(stmt);

	return v;
}
