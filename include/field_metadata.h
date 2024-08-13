#pragma once

#include <stddef.h>

typedef struct {
	const char* placeholder;
	size_t offset;
	const char* type;
} field_metadata;

#define FIELD_METADATA(struct_type, field, type) { #field, offsetof(struct_type, field), type }

#define METADATA_TERMINATOR {NULL, 0, NULL}