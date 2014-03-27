#include "ctype.h"

#include "util.h"

char *read_line(FILE *f, bool downcase, bool trim) {
	char read_buffer[512] = {0};
	char *line = NULL;
	char *rc = NULL;

	// Sanity check the stream before we go on,
	check_debug(feof(f) == 0, "FILE(%p) is at EOF", f);
	check(ferror(f) == 0, "Error in stream(%p).", f);

	// Try reading, with the hope that we get the entire line at once
	rc = fgets(read_buffer, sizeof(read_buffer), f);
	check(rc != NULL, "Failed to read line from FILE(%p)", f);
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
			log_err("TODO: trim not done");
		}

		if(downcase == true) {
			log_err("TODO: downcase not done.");
		}
	}

	return line;
error:
	if(line != NULL) free(line);
	if(feof(f)) debug("FILE(%p) EOF", f);
	if(ferror(f)) debug("FILE(%p): ERROR. %s", f, clean_errno());
	return NULL;
}
