#include "server.h"
#include "global.h"
#include "database.h"

int main() {
	
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