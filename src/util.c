#include "ctype.h"

#include "util.h"

char *str_dup(char *str) {
	char *copy_str = NULL;
	check_mem(copy_str = calloc(strlen(str) + 1, sizeof(char)));
	strncpy(copy_str, str, strlen(str));
	return copy_str;
error:
	return NULL;
}

char *str_ltrim(char *str, bool copy) {
	if(!copy) {
		// Find the first non-space char in the string
		char *start = str;
		while(*start && isspace(*start)) {
			start++;
		}

		// Do nothing if the string starts on a sane char
		if(start != str) {
			// Shift the string and NULL-cap it if we found a subset
			// otherwise NULL the head of the string
			if((start - str) != strlen(str)) {
				memmove(str, start, strlen(start));
				str[strlen(start)] = '\0';
			}
			else {
				str[0] = '\0';
			}
		}
		return str;
	}
	else {
		char *copy_str = NULL;
		check_mem(copy_str = str_dup(str));
		return str_ltrim(copy_str, false);
	}
error:
	return NULL;
}

char *str_rtrim(char *str, bool copy) {
	if(!copy) {
		char *end = str + strlen(str) - 1;
		while((end > str) && isspace(*end)) {
			*end-- = '\0';
		}
		return str;
	}
	else {
		char *copy_str = NULL;
		check_mem(copy_str = str_dup(str));
		return str_ltrim(copy_str, false);
	}
error:
	return NULL;
}

char *str_trim(char *str, bool copy) {
	char *trim_str = copy ? str_dup(str) : str;
	check_mem(trim_str);

	// Since we have already made a copy if we need one,
	// we can just chain these two together, since non-copy
	// operations can't throw out a NULL and the pointer itself
	// cannot change.
	return str_ltrim(str_rtrim(trim_str, false), false);
error:
	return NULL;
}

char *read_line(FILE *f, bool downcase, bool trim) {
	char read_buffer[512] = {0};
	char *line = NULL;
	char *rc = NULL;

	// Sanity check the stream before we go on,
	// an EOF here is not fatal, so we'll return early
	// instead of jumping into the error machinery.
	if(feof(f) != 0) return NULL;
	check(ferror(f) == 0, "Error in stream(%p).", f);

	// Try reading, with the hope that we get the entire line at once.
	// Short circuit EOF, so we don't emit noise if we can avoid it.
	rc = fgets(read_buffer, sizeof(read_buffer), f);
	if((rc == NULL) && feof(f)) return NULL;

	check_debug(rc != NULL, "Failed to read line from FILE(%p)", f);
	check_mem(line = calloc(strlen(read_buffer) + 1, sizeof(char)));
	strncpy(line, read_buffer, strlen(read_buffer));

	// See if we need to finish reading the line
	while(line[strlen(line) - 1] != '\n') {
		rc = fgets(read_buffer, sizeof(read_buffer), f);
		if((rc == NULL) && feof(f)) {
			// We got everything that we can get, so we'll
			// call it a "line"
			break;
		}
		else {
			// Append the new data to the end of the line
			char *new_line = NULL;
			check(rc != NULL, "Error finishing line from FILE(%p)", f);
			check_mem(new_line = calloc(strlen(line) + strlen(read_buffer) + 1, sizeof(char)));

			strncpy(new_line, line, strlen(line));
			strncpy(new_line + strlen(new_line), read_buffer, strlen(read_buffer));

			free(line);
			line = new_line;
		}
	}

	// Post processing
	if((line != NULL) && (strlen(line) > 0)) {
		if(trim == true) {
			line = str_trim(line, false);
		}

		if(downcase == true) {
			for(int i = 0; i < strlen(line); i++) {
				line[i] = tolower(line[i]);
			}
		}
	}

	return line;
error:
	if(line != NULL) free(line);
	if(feof(f)) debug("FILE(%p) EOF", f);
	if(ferror(f)) debug("FILE(%p): ERROR. %s", f, clean_errno());
	return NULL;
}

char *next_line(FILE *f, bool downcase, bool trim) {
	char *line = NULL;

	// Read lines until we get one that isn't blank,
	while(((line = read_line(f, downcase, trim)) != NULL) &&
		  (strlen(line) == 0)) {
		free(line);
		line = NULL;
	}

	return line;
}
