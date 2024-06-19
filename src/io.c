#include "io.h"
#include <stdlib.h>
#include <string.h>

#define IO_READ_CHUNK_SIZE 2097152
#define IO_READ_ERROR_GENERAL "Error reading file: %s\n"
#define IO_READ_ERROR_MEMORY "Not enough free memory to read file: %s\n"

File io_read_file(const char* path)
{
	File file = { .is_valid = false,
		.path = path};

	FILE* fp = fopen(path, "rb");

	if (!fp || ferror(fp)) {
		printf(IO_READ_ERROR_GENERAL, path);
		return file;
	}

	char* data = NULL;
	char* temp;
	size_t used = 0;
	size_t size = 0;
	size_t n;

	while (true) {
		if (used + IO_READ_CHUNK_SIZE + 1 > size) {
			size = used + IO_READ_CHUNK_SIZE + 1;

			if (size <= used) {
				free(data);
				printf("File is too large to read: %s\n", path);
				return file;
			}

			temp = realloc(data, size);
			if (!temp) {
				free(data);
				printf(IO_READ_ERROR_MEMORY, path);
				return file;
			}

			data = temp;
		}

		n = fread(data + used, 1, IO_READ_CHUNK_SIZE, fp);
		if (n == 0) {
			break;
		}
		used += n;
	}

	if (ferror(fp)) {
		printf(IO_READ_ERROR_GENERAL, path);
		return file;
	}

	temp = realloc(data, used + 1);
	if (!temp) {
		free(data);
		printf(IO_READ_ERROR_MEMORY, path);
		return file;
	}

	fclose(fp);

	data = temp;
	data[used] = 0;

	file.data = data;
	file.len = used;
	file.is_valid = true;


	return file;
}

int io_file_write(void* buffer, size_t size, const char* path)
{
	FILE* fp = fopen(path, "wb");

	if (!fp || ferror(fp)) {
		printf("Failed to open file: %s\n", path);
		return 1;
	}

	size_t chunks_written = fwrite(buffer, size, 1,fp);

	fclose(fp);

	if (chunks_written != 1) {
		printf("Write error. Expected 1 chunk, got %zu", chunks_written);
		return 1;
	}

	return 0;
}

void io_file_append(void* buffer, size_t size, const char* path)
{
	FILE* fp = fopen(path, "ab");

	if (!fp || ferror(fp)) {
		printf("Failed to open file: %s\n", path);
		return;
	}

	fwrite(buffer, 1, size, fp);

	fclose(fp);
}

void io_file_readline(char* buffer, size_t line, const char* path) {
	FILE* fp = fopen(path, "rb");

	if (!fp || ferror(fp)) {
		printf("Failed to open file: %s\n", path);
		return;
	}

	size_t i = 0;
	int j = 0;

	while (i != line) {
		char* temp = malloc(1);
		fread(temp, 1, 1, fp);
		if (temp[0] == '\n')
			i++;
		if(i == line - 1 && temp[0] != '\n') {
			memcpy(buffer + j, temp, 1);
			j++;
		}
		memset(buffer + j, '\0', 1);

		free(temp);
	}

	fclose(fp);
}

size_t io_file_count_lines(const char* path)
{
	size_t lines = 0;

	FILE* fp = fopen(path, "rb");

	if (!fp || ferror(fp)) {
		printf(IO_READ_ERROR_GENERAL, path);
		return -1;
	}
	
	while (!feof(fp))
	{
		char ch = fgetc(fp);
		if (ch == '\n')
		{
			lines++;
		}
	}

	return lines;
}
