#include <stdio.h>
#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "bsp.h"
#include "export.h"

int main(int argc, char **argv) {
	char *file = NULL;
	stl_object *file_stl = NULL;
	bsp_node_t *file_bsp = NULL;
	klist_t(poly) *polygons = kl_init(poly);
	check(argc >= 2, "Need a filename");

	file = argv[1];
	file_stl = stl_read_file(file, 0);
	check(file_stl != NULL, "Failed to read stl from '%s'", file);
	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);

	file_bsp = stl_to_bsp(file_stl);
	check_mem(file_bsp);

	stl_object *out = bsp_to_stl(file_bsp);
	check(stl_write_file(out, "/tmp/out.stl") == 0, "Failed to write STL");


	kl_destroy(poly, polygons);
	stl_free(file_stl);
	log_info("Terminating Success");
	return 0;
error:
	kl_destroy(poly, polygons);
	if(file_stl != NULL) stl_free(file_stl);
	log_err("Terminating Failure");
	return -1;
}
