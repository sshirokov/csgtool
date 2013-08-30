#include <string.h>
#include "dbg.h"

#include "commands.h"
#include "csg_commands.h"
#include "stl.h"
#include "bsp.h"
#include "export.h"
#include "index.h"
#include "filter.h"


int cmd_clean(int argc, char **argv) {
	char *out = "./out.cleaned.stl";
	stl_object *stl = NULL;
	stl_object *cleaned = NULL;
	klist_t(poly) *polys = NULL;
	klist_t(poly) *filtered = NULL;
	mesh_index_t *index = NULL;
	int rc = -1;
	check(argc >= 1, "An input file is required.");
	check((stl = stl_read_file(argv[0], 1)) != NULL, "Failed to read file from '%s'", argv[0]);
	check((polys = stl_to_polys(stl)) != NULL, "Failed to get polygon list from %p (%s)", stl, argv[0]);

	// Replace the output path if we were given one
	if(argc > 2) out = argv[1];

	// Perform a simple filter to remove polys with singularity edges
	filtered = filter_polys(NULL, polys, filter_test_edge_singularity);
	// TODO: check()s
	log_info("Filter result %zd => %zd polygons", polys->size, filtered->size);
	kl_destroy(poly, polys);
	polys = filtered;

	// Create an index for the mesh
	index = alloc_mesh_index(polys);
	check(index != NULL, "Failed to generate index of %zd polygons from %s", polys->size, argv[0]);

	// Remove bisecting vertecies by adding polygons
	filtered = map_polys_with_index(index, NULL, polys, map_bisect_to_triangles);
	log_info("%zd mapped polys produced with index map.", filtered->size);
	// TODO: check()
	free_mesh_index(index);
	kl_destroy(poly, polys);
	polys = filtered;

	// Reindex the new set of polygons
	index = alloc_mesh_index(filtered);

	// Walk the polygons and only keep ones that return more than two neighbors
	log_info("Removing polygons with too few neighbors.");
	klist_t(poly) *real_polys = kl_init(poly);
	// TODO: check
	kliter_t(idx_poly) *iter = kl_begin(index->polygons);
	for(; iter != kl_end(index->polygons); iter = kl_next(iter)) {
		idx_poly_t *poly = kl_val(iter);
		klist_t(idx_poly) *neighbors = index_find_poly_neighbors(index, poly);
		check(neighbors != NULL, "Failed to find neighbors of poly in clean.");
		if(neighbors->size > 1) {
			*kl_pushp(poly, real_polys) = poly->poly;
		}
		kl_destroy(idx_poly, neighbors);
	}

	// Write the result
	log_info("Result has %zd polys, writing to %s", real_polys->size, out);
	cleaned = stl_from_polys(real_polys);
	rc = stl_write_file(cleaned, out);
	log_info("Write result: %d", rc);

	if(index != NULL) free_mesh_index(index);
	if(polys != NULL) kl_destroy(poly, polys);
	if(stl != NULL) stl_free(stl);
	return 0;
error:
	if(index != NULL) free_mesh_index(index);
	if(polys != NULL) kl_destroy(poly, polys);
	if(stl != NULL) stl_free(stl);
	return -1;
}

// Available commands
const cmd_t commands[] = {
	{"intersect", "Intersect two geometries", cmd_intersect},
	{"subtract",  "Subtract two geometries",  cmd_subtract},
	{"union",     "Union two geometries",     cmd_union},
	{"clean",     "Compute a de-duplicated v<->p index and clean polygons without neighbors", cmd_clean},
	{NULL, NULL, NULL}
};

// Search for a command by name.
cmd_fun_t cmd_find(const char *name) {
	for(cmd_t *c = (cmd_t*)commands; c->name != NULL; c++) {
		if(0 == strcmp(c->name, name)) return c->fun;
	}
	return NULL;
}
