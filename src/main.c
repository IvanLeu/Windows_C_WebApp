#include "server.h"
#include "global.h"
#include "database.h"

#include "hash_table.h"
#include "vector.h"
#include "users.h"

int main() {

	{
		User user;
		user.name = "dsadsa";
		user.password = "dsadsad";
		user.email = "dsadsadas";
		user.id = 1;

		Vector* users = vector_create(sizeof(User), 10);
		vector_push_back(users, &user);

		HashTable* ht = hash_table_create();

		int arr_size = users->capacity * users->item_size;

		hash_table_insert(ht, "users", users, arr_size);

		Vector* v = malloc(arr_size);
		memcpy(v, hash_table_at(ht, "users"), arr_size);
		User* u = vector_at(v, 0);

		while (true);
	}

	sqlite3* db;
	database_connect(&db);
	global.db = db;

	Server* server;
	server = server_init(8080);
	server_listen(server);
	server_shutdown(server);

	database_close(db);

	return 0;
}