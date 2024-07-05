#pragma once
#include <stdbool.h>
#include <stdio.h>

typedef struct File {
	char* data;
	size_t len;
	bool is_valid;
	const char* path;
} File;

File io_read_file(const char* path);
int io_file_write(void* buffer, size_t size, const char* path);
void io_file_append(void* buffer, size_t size, const char* path);
void io_file_readline(char* buffer, size_t line, const char* path);
size_t io_file_count_lines(const char* path);