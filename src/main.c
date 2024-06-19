#include "server.h"
#include "global.h"

int main() {
	global.user_db = user_database_init("users.txt");

	Server* server;
	server = server_init(8080);
	server_listen(server);
	server_shutdown(server);

	return 0;
}