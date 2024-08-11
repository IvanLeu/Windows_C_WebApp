#pragma once

#define MAX_TABLE_SIZE 1024

typedef enum {
	VAL_INT,
	VAL_FLOAT,
	VAL_STRING,
	VAL_ARRAY
} Value_Type; // probably add more later

typedef struct value {
	Value_Type type;
	void* data;
} HT_Value;

typedef struct Item {
	char* key;
	HT_Value* value;
	struct Item* next;
} Item;

typedef struct hash_table {
	Item* items[MAX_TABLE_SIZE];
} HashTable;

HashTable* hash_table_create();
void hash_table_insert(HashTable* ht, const char* key, Value_Type value_type, void* value, size_t val_size);
HT_Value* hash_table_at(HashTable* ht, const char* key);
void hash_table_delete(HashTable** ht);
void hash_table_print(HashTable* ht);
int hash(const char* key);