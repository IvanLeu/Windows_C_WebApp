#include "vector.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

Vector* vector_create(size_t item_size, size_t capacity) {
	Vector* v = malloc(sizeof(Vector));

	if (!v) {
		printf("Failed to allocate memory for vector");
		exit(1);
	}

	if (capacity < 0) {
		v->capacity = 0;
	}
	v->capacity = capacity;

	v->size = 0;
	v->item_size = item_size;
	v->data = malloc(v->capacity * v->item_size);

	if (!v->data) {
		printf("Failed to allocate memory for vector");
		exit(1);
	}

	return v;
}

void vector_push_back(Vector* v, void* item) {
	if (v->size == v->capacity) {
		v->capacity = v->capacity > 0 ? v->capacity * 2 : 1;
		void* items = realloc(v->data, v->capacity * v->item_size);

		if (!items) {
			printf("Failed to allocate memory for vector");
			exit(1);
		}

		v->data = items;
	}

	size_t index = v->size++;

	memcpy((uint8_t*)v->data + index * v->item_size, item, v->item_size);

}

void vector_erase(Vector* v, size_t index) {
	assert(0 <= index && index <= v->size - 1);
	
	if (v->size == 0) {
		return;
	}

	if (v->size == 1) {
		v->size = 0;
		return;
	}

	if (index == v->size - 1) {
		v->size--;
		return;
	}

	--v->size;

	uint8_t* item_ptr = (uint8_t*)v->data + index * v->item_size;
	uint8_t* end_ptr = (uint8_t*)v->data + v->size * v->item_size;
	memcpy(item_ptr, end_ptr, v->item_size);

	return;
}

void* vector_at(Vector* v, size_t index) {
	assert(0 <= index && index <= v->size - 1);

	return (uint8_t*)v->data + index * v->item_size;
}

size_t vector_size(Vector* v) {
	return v->size;
}

bool vector_empty(Vector* v) {
	return v->size == 0;
}

void vector_destroy(Vector** v) {
	free((*v)->data);
	free(*v);
	(*v) = NULL;
	return;
}
