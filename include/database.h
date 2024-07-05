#pragma once

#include "sqlite3\sqlite3.h"

void database_connect(sqlite3** db);
void database_close(sqlite3* db);
