#pragma once
#include "user_database.h"

typedef struct global {
	UserDatabase user_db;
	size_t last_id;
} Global;

extern Global global;