#pragma once

#define MAX_TABLE_SIZE 1024

typedef struct Item {
	char* key;
	void* value;
	struct Item* next;
} Item;

typedef struct hash_table {
	Item* items[MAX_TABLE_SIZE];
} HashTable;

HashTable* hash_table_create();
void hash_table_insert(HashTable* ht, const char* key, void* value, size_t val_size);
void* hash_table_at(HashTable* ht, const char* key);
void hash_table_delete(HashTable** ht);
void hash_table_print(HashTable* ht);
int hash(const char* key);