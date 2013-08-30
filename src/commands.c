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
	klist_t(poly) *polys = NULL;
	mesh_index_t *index = NULL;
	check(argc >= 1, "An input file is required.");
	check((stl = stl_read_file(argv[0], 1)) != NULL, "Failed to read file from '%s'", argv[0]);
	check((polys = stl_to_polys(stl)) != NULL, "Failed to get polygon list from %p (%s)", stl, argv[0]);

	// Replace the output path if we were given one
	if(argc > 2) out = argv[1];

	// Perform a simple filter to remove polys with singularity edges
	klist_t(poly) *filtered = filter_polys(NULL, polys, filter_test_edge_singularity);
	log_info("Filter result %zd/%zd", polys->size, filtered->size);
	// TODO: Don't do this, but also rewrite this whole method to not be a test driver.

	stl_object *f_stl = stl_from_polys(filtered);
	log_info("Wrtiting filterd: %d", stl_write_file(f_stl, "./out.filtered.stl"));
	stl_free(f_stl);
	kl_destroy(poly, polys);
	polys = filtered;

	index = alloc_mesh_index(polys);
	size_t verts = 0;
	size_t edges = 0;
	check(index != NULL, "Failed to generate index of %zd polygons from %s", polys->size, argv[0]);
	vertex_tree_walk(index->vertex_tree, vertex_node_count, &verts);
	edge_tree_walk(index->edge_tree, edge_node_count, &edges);
	log_info("Polys in index: %zd Verts in index: %zd Edges in index: %zd", index->polygons->size, verts, edges);

	klist_t(poly) *real_polys = kl_init(poly);

	// Walk the polygons and only keep ones that return more than two neighbors
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

	log_info("Result has %zd polys, writing to %s", real_polys->size, out);
	stl_object *cleaned = stl_from_polys(real_polys);
	int rc = stl_write_file(cleaned, out);
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
