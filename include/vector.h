#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct Vector {
	size_t capacity;
	size_t size;
	size_t item_size;
	void* data;
} Vector;

Vector* vector_create(size_t item_size, size_t capacity);
void vector_push_back(Vector* v, void* item);
void vector_erase(Vector* v, size_t index);
void* vector_at(Vector* v, size_t index);
size_t vector_size(Vector* v);
bool vector_empty(Vector* v);
void vector_destroy(Vector** v);