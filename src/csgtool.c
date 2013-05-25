#include <stdio.h>

#include "dbg.h"

#include "stl.h"

int main(int argc, char **argv) {
	check(argc >= 2, "Need a filename");
	char *file = argv[1];
	stl_object *file_stl = stl_read_file(file);
	check(file_stl != NULL, "Failed to read stl from '%s'", file);

	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);


	stl_free(file_stl);
	return 0;
error:
	if(file_stl != NULL) stl_free(file_stl);
	return -1;
}
