#include "database.h"
#include "users.h"
#include "session.h"
#include <stdio.h>
#include <stdlib.h>

void database_connect(sqlite3** db)
{
	int rc = sqlite3_open("database.db", db);
	if (rc != SQLITE_OK) {
		printf("Error msg: %s", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	// create required tables
	create_users_table(*db);
	create_sessions_table(*db);
}

void database_close(sqlite3* db)
{
	sqlite3_close(db);
}
