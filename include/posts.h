#pragma once

#include <time.h>
#include "users.h"

typedef enum Post_Query_Type {
	Query_None = 0,
	Query_By_ID = 1,
	Query_By_Title = 2,
	Query_By_Author_ID = 3,
	Query_By_Date = 4,
	Query_All = 5,
} Post_Query_Type;

typedef struct {
	size_t id;
	char* title;
	char* content;
	User* author;
	time_t date;
} Post;

Post* create_post();
void delete_post(Post* post);

void create_posts_table(sqlite3* db);
void insert_post(sqlite3* db, const char* title, const char* content, User* author, time_t date);
Vector* query_post(sqlite3* db, Post_Query_Type query_type, const char* key);