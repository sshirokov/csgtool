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

	stl_object *file2_stl = NULL;
	bsp_node_t *file2_bsp = NULL;
	check(argc >= 3, "Need two filename");

	// Read file 1
	file = argv[1];
	file_stl = stl_read_file(file, 0);
	check(file_stl != NULL, "Failed to read stl from '%s'", file);
	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);

	file_bsp = stl_to_bsp(file_stl);
	check_mem(file_bsp);

	// Read file 2
	file = argv[2];
	file2_stl = stl_read_file(file, 0);
	check(file2_stl != NULL, "Failed to read stl from '%s'", file);
	log_info("Loaded file2: %s %d facets", file, file2_stl->facet_count);

	file2_bsp = stl_to_bsp(file2_stl);
	check_mem(file2_bsp);

	// Do something
	log_info("Invert: %p", bsp_invert(file_bsp));
	log_info("Clip: %p", bsp_clip(file2_bsp, file_bsp));
	log_info("Invert2: %p", bsp_invert(file2_bsp));
	log_info("Clip2: %p", bsp_clip(file_bsp, file2_bsp));
	log_info("Clip3: %p", bsp_clip(file2_bsp, file_bsp));
	klist_t(poly) *tree2_polys = bsp_to_polygons(file2_bsp, 0, NULL);
	log_info("Got %zd polys from tree2", tree2_polys->size);
	bsp_node_t *result = bsp_build(file_bsp, tree2_polys);
	log_info("Invert: %p", bsp_invert(result));
	log_info("Bsp node result: %p", result);

	stl_object *out = bsp_to_stl(result);
	check(stl_write_file(out, "/tmp/out.stl") == 0, "Failed to write STL");


	stl_free(file_stl);
	stl_free(file2_stl);
	log_info("Terminating Success");
	return 0;
error:
	if(file_stl != NULL) stl_free(file_stl);
	if(file_stl != NULL) stl_free(file2_stl);
	log_err("Terminating Failure");
	return -1;
}
