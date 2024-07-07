#pragma once
#include "sqlite3\sqlite3.h"

typedef struct global {
	sqlite3* db;
	size_t last_id;
} Global;

extern Global global;