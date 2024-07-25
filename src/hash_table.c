#include "hash_table.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static Item* item_create(const char* key, void* val, size_t val_size) {
	Item* item = malloc(sizeof(Item));
	item->key = malloc(strlen(key) + 1);
	item->value = malloc(val_size);

	strcpy(item->key, key);
	memcpy(item->value, val, val_size);
	item->next = NULL;

	return item;
}

HashTable* hash_table_create() {
	HashTable* ht = malloc(sizeof(HashTable));

	for (int i = 0; i < MAX_TABLE_SIZE; i++) {
		ht->items[i] = NULL;
	}

	return ht;
}

int hash(const char* key) {
	if (!key) {
		fprintf(stderr, "Invalid key passed");
		return -1;
	}

	const int p = 31;
	const int m = 1e6 + 9;
	long long hash_value = 0;
	long long p_pow = 1;

	for (int i = 0; key[i] != '\0'; i++) {
		hash_value = (hash_value + (key[i] - 'a' + 1) * p_pow) % m;
		p_pow = (p_pow * p) % m;
	}
	return hash_value % MAX_TABLE_SIZE;
}

void hash_table_insert(HashTable* ht, const char* key, void* value, size_t val_size) {
	if (ht == NULL)
		return;

	int index = hash(key);

	Item* item = ht->items[index];

	if (item == NULL) {
		ht->items[index] = item_create(key, value, val_size);
		return;
	}

	Item* prev;

	while (item != NULL) {
		if (strcmp(item->key, key) == 0) {
			free(item->value);
			item->value = malloc(val_size);
			memcpy(item->value, value, val_size);
			return;
		}
		prev = item;
		item = prev->next;
	}

	prev->next = item_create(key, value, val_size);
}

void* hash_table_at(HashTable* ht, const char* key) {
	if (ht == NULL)
		return NULL;
	
	int index = hash(key);

	Item* item = ht->items[index];

	if (item == NULL)
		return NULL;

	while (item != NULL) {
		if (strcmp(item->key, key) == 0) {
			return item->value;
		}
		item = item->next;
	}

	return NULL;
}

static void item_free(Item* item) {
	if (item == NULL) {
		return;
	}

	if (item->next == NULL) {
		free(item);
		return;
	}
	item_free(item->next);
	free(item);
}

void hash_table_delete(HashTable** ht)
{
	if ((*ht) == NULL)
		return;

	for (int i = 0; i < MAX_TABLE_SIZE; i++) {
		item_free((*ht)->items[i]);
	}
	free(*ht);
	*ht = NULL;
}

void hash_table_print(HashTable* ht)
{
	printf("\n");

	for (int i = 0; i < MAX_TABLE_SIZE; i++) {
		if (ht->items[i] == NULL)
			continue;
		printf("%s : %p\n", ht->items[i]->key, ht->items[i]->value);
	}

	printf("\n");
}
