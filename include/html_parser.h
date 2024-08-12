#pragma once

#include "io.h"
#include "hash_table.h"

char* include_html(File* file);
void process_template_data(char** pp_data, HashTable* ht);