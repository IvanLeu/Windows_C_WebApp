#include "../include/html_parser.h"

#include "../include/vector.h"
#include "../include/utils.h"
#include "../include/field_metadata.h"
#include <stdlib.h>
#include <inttypes.h>

#define MAX_BUFFER_SIZE 2048

char* include_html(File* file) {
	char* file_data = malloc(strlen(file->data) + 1);

	if (!file_data) {
		return NULL;
	}

	strcpy(file_data, file->data);

	char* begin = strstr(file_data, "{%");
	if (!begin) {
		return NULL;
	}
	begin += strlen("{%");

	char* end = strstr(begin, "%}");
	if (!end) {
		return NULL;
	}

	*end = '\0';

	char* keyword = strstr(begin, "include");

	if (!keyword) {
		return NULL;
	}

	keyword += strlen("include");

	char include_file_name[128] = { 0 };
	for (int i = 0; keyword != end; keyword++) {
		if (keyword[0] != ' ') {
			include_file_name[i++] = keyword[0];
		}
	}

	char* path = malloc(256);
	if (!path) {
		return NULL;
	}

	sprintf(path, "./src/templates/%s", include_file_name);

	File include_file;
	include_file = io_read_file(path);
	free(path);

	char* file_prefix_end = strstr(include_file.data, "{% block %}");
	if (!file_prefix_end)
		return NULL;

	char* file_suffix_begin = strstr(file_prefix_end, "{% endblock %}");
	if (!file_suffix_begin)
		return NULL;
	file_suffix_begin += strlen("{% endblock %}");

	char* file_suffix_end = strstr(file_suffix_begin, "</html>");
	if (!file_suffix_end)
		return NULL;
	file_suffix_end += strlen("</html>\r\n\r\n");

	char* block_begin = strstr(file->data, "{% block %}");
	if (!block_begin)
		return NULL;
	block_begin += strlen("{% block %}");

	char* block_end = strstr(block_begin, "{% endblock %}");
	if (!block_end)
		return NULL;

	int out_file_size = (file_prefix_end - include_file.data) + (block_end - block_begin) + (file_suffix_end - file_suffix_begin);
	char* send_data = malloc(out_file_size);
	if (!send_data)
		return NULL;

	memcpy(send_data, include_file.data, file_prefix_end - include_file.data);
	memcpy(send_data + (file_prefix_end - include_file.data), block_begin, block_end - block_begin);
	memcpy(send_data + (file_prefix_end - include_file.data) + (block_end - block_begin),
		file_suffix_begin, file_suffix_end - file_suffix_begin);

	*(send_data + out_file_size) = '\0';

	return send_data;
}

static const char* get_fieldname_from_object(const char* begin_statement, const char* end_statement, const char* object_name) {
	char* result = NULL;
	// assume that begin/end of statement is valid
	begin_statement += strlen("{{");
	end_statement -= strlen("}}");

	const int body_size = end_statement - begin_statement;
	char* body = malloc(body_size + 1);
	memcpy(body, begin_statement, body_size);
	body[body_size] = 0;

	// to free later
	char* bodycpy = body;

	remove_spaces(body);

	//no field access or invalid object name
	if (strlen(object_name) >= strlen(body)) {
		free(bodycpy);
		return result;
	}

	//compare the prefixes
	if (memcmp(object_name, body, strlen(object_name)) == 0) {
		const int result_size = body_size - strlen(object_name) - 1;
		result = malloc(result_size + 1);
		memcpy(result, body + strlen(object_name) + 1, result_size);
		result[result_size] = 0;
	}

	free(bodycpy);
	return result;
}

void process_template_data(char** pp_data, HashTable* ht) {
	char* data = *pp_data;
	// process if statements
	// conditions will be formed as a single variable and will be
	// true if variable is not NULL to keep things simple
	while (strstr(data, "{% IF") != NULL) {
		const char* if_start = strstr(data, "{% IF");
		if (!if_start) break;
		const char* if_body_start = strstr(if_start, "%}");
		if (!if_body_start) break;
		if_body_start += strlen("%}");
		const char* if_end = strstr(if_start, "{% ENDIF %}");
		if (!if_end) break;
		if_end += strlen("{% ENDIF %}");

		const char* if_body_end = NULL;
		const char* else_body_start = NULL;
		const char* else_body_end = NULL;

		// get the heyword
		const int keyword_size = (if_body_start - strlen("%}") - (if_start + strlen("{% IF")));
		char* const keyword = malloc(keyword_size + 1);
		memcpy(keyword, (if_start + strlen("{% IF")), keyword_size);
		keyword[keyword_size] = 0;

		remove_spaces(keyword);

		// determine the rest of pointers based of existence of else statement
		if (strstr(if_body_start, "{% ELSE %}") && strstr(if_body_start, "{% ELSE %}") < strstr(if_body_start, "{% ENDIF %}")) {
			if_body_end = strstr(if_body_start, "{% ELSE %}");
			else_body_start = if_body_end + strlen("{% ELSE %}");
			else_body_end = strstr(else_body_start, "{% ENDIF %}");
		}
		else {
			if_body_end = strstr(if_body_start, "{% ENDIF %}");
		}

		//whole if statement body
		const int full_if_size = if_end - if_start;
		char* full_if = malloc(full_if_size + 1);
		memcpy(full_if, if_start, full_if_size);
		full_if[full_if_size] = 0;

		//choose branch based on the keyword
		if (hash_table_at(ht, keyword)) {
			const int if_body_size = if_body_end - if_body_start;
			char* if_body = malloc(if_body_size + 1);
			memcpy(if_body, if_body_start, if_body_size);
			if_body[if_body_size] = 0;

			*pp_data = str_replace(data, full_if, if_body);
			//maybe free data??
			data = *pp_data;
			free(if_body);
		}
		else {
			if (else_body_start && else_body_end) {
				const int else_body_size = else_body_end - else_body_start;
				char* else_body = malloc(else_body_size + 1);
				memcpy(else_body, else_body_start, else_body_size);
				else_body[else_body_size] = 0;

				*pp_data = str_replace(data, full_if, else_body);
				//maybe free data??
				data = *pp_data;
				free(else_body);
			}
			else {
				*pp_data = str_replace(data, full_if, " ");
				//maybe free data??
				data = *pp_data;
			}
		}

		free(keyword);
		free(full_if);
	}

	// proccessing for loops
	while (strstr(data, "{% FOR")) {
		const char* loop_start = strstr(data, "{% FOR");
		if (!loop_start) break;
		const char* body_start = strstr(loop_start, "%}");
		if (!body_start) break;
		body_start += strlen("%}");
		const char* body_end = strstr(body_start, "{% ENDFOR %}");
		if (!body_end) break;
		const char* loop_end = body_end + strlen("{% ENDFOR %}");

		//extract loop condition data
		const int temp_size = (body_start - strlen("%}")) - (loop_start + strlen("{% FOR"));
		char* temp = malloc(temp_size + 1);
		memcpy(temp, loop_start + strlen("{% FOR"), temp_size);
		temp[temp_size] = 0;
		char* temp_cpy = temp;
		char* const item_name = strtok_s(temp, ":", &temp);
		char* const array_name = strtok_s(temp, ":", &temp);

		remove_spaces(item_name);
		remove_spaces(array_name);

		//get the full for loop body before proccessing
		const int before_loop_size = loop_end - loop_start;
		char* before_loop = malloc(before_loop_size + 1);
		memcpy(before_loop, loop_start, before_loop_size);
		before_loop[before_loop_size] = 0;

		Vector* v = hash_table_at(ht, array_name)->data;
		if (!v) {
			*pp_data = str_replace(data, before_loop, " ");
			//maybe free data??
			data = *pp_data;
			continue;
		}

		const int body_size = body_end - body_start;
		char* body = malloc(body_size + 1);

		memcpy(body, body_start, body_size);
		body[body_size] = 0;

		const int new_body_size = body_size * vector_size(v);
		char* new_body = malloc(new_body_size + 1);
		for (int i = 0; i < vector_size(v); i++) {
			char* temp = new_body + body_size * i;

			//replace vars with values
			const char* begin_statement;
			while ((begin_statement = strstr(body, "{{")) != NULL) {
				const char* end_statement = strstr(begin_statement, "}}");
				if (!end_statement) {
					break;
				}
				end_statement += strlen("}}");
				//whole statement to replace later
				const int statement_size = end_statement - begin_statement;
				char* statement = malloc(statement_size + 1);
				memcpy(statement, begin_statement, statement_size);
				statement[statement_size] = 0;

				// basically works like this: "{{ item_name.field }}" -> returns "field"
				const char* field_name = get_fieldname_from_object(begin_statement, end_statement, item_name);
				if (!field_name) {
					continue;
				}
				//every struct that can be stored in array should have metadata defined as FIRST field
				void* current_item = vector_at(v, i);
				//problem here
				const field_metadata* metadata = (field_metadata*)malloc(sizeof(field_metadata));
				memcpy(metadata, current_item, sizeof(field_metadata*));

				for (int j = 0; metadata[j].type != NULL; j++) {
					char* MD_field_name = metadata[j].placeholder;
					size_t MD_offset = metadata[j].offset;
					char* MD_type = metadata[j].type;

					char replace_with[MAX_BUFFER_SIZE];
					if (metadata[j].type == "string") {
						sprintf(replace_with, "%s", (char*)((char*)current_item + MD_offset));
					}
					else if (metadata[j].type == "int") {
						sprintf(replace_with, "%d", *((int*)((char*)current_item + MD_offset)));
					}
					else if (metadata[j].type == "float") {
						sprintf(replace_with, "%f", *((float*)((char*)current_item + MD_offset)));
					}
					else if (metadata[j].type == "size_t") {
						sprintf(replace_with, "%zu", *((uint64_t*)current_item + MD_offset));
					}
					else {
						free(statement);
						free(metadata);
						continue;
					}

					str_replace(temp, statement, replace_with);
					free(statement);
					free(metadata);
				}
				
			}

			memcpy(temp, body, body_size);
		}
		new_body[new_body_size] = 0;

		*pp_data = str_replace(data, before_loop, new_body);
		//maybe free data??
		data = *pp_data;

		free(body);
		free(new_body);
		free(before_loop);
		free(temp_cpy);
	}

	// last step: replace remaining {{ }} variables
	// ONLY string replacement - other types will be ignored
	while (strstr(data, "{{")) {
		const char* const begin = strstr(data, "{{");
		const char* const end = strstr(begin, "}}") + strlen("}}");

		const int statement_size = end - begin;
		char* const to_replace = malloc(statement_size + 1);
		memcpy(to_replace, begin, statement_size);
		to_replace[statement_size] = 0;

		const char* const body_begin = begin + strlen("{{");
		const char* const body_end = end - strlen("}}");
		const int body_size = body_end - body_begin;
		char* const body = malloc(body_size + 1);
		memcpy(body, body_begin, body_size);
		body[body_size] = 0;

		//to deallocate later
		const char* bodycpy = body;

		remove_spaces(body);

		HT_Value* val = hash_table_at(ht, body);
		if (val != NULL) {
			char with[2048]; //probably enough
			switch (val->type) {
			case VAL_INT:
				sprintf(with, "%d", *((int*)val->data));
				break;
			case VAL_FLOAT:
				sprintf(with, "%f", *((float*)val->data));
				break;
			case VAL_STRING:
				sprintf(with, "%s", (char*)val->data);
				break;
			}

			*pp_data = str_replace(data, to_replace, with);
			data = *pp_data;

		}

		//otherwise just deallocate and move on;
		free(to_replace);
		free(bodycpy);
	}
}