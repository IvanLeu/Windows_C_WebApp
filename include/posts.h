#pragma once

#include <time.h>
#include "users.h"
#include "field_metadata.h"

typedef enum Post_Query_Type {
	Posts_Query_None = 0,
	Posts_Query_By_ID = 1,
	Posts_Query_By_Title = 2,
	Posts_Query_By_Author_ID = 3,
	Posts_Query_By_Date = 4,
	Posts_Query_All = 5,
} Post_Query_Type;

typedef struct {
	field_metadata* metadata;
	size_t id;
	char* title;
	char* content;
	User* author;
	char* creation_date;
} Post;

extern field_metadata post_metadata[];

Post* create_post();
void delete_post(Post* post);

void create_posts_table(sqlite3* db);
void insert_post(sqlite3* db, const char* title, const char* content, User* author);
Vector* query_post(sqlite3* db, Post_Query_Type query_type, const char* key);