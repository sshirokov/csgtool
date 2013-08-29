#include <string.h>
#include "dbg.h"

#include "commands.h"
#include "stl.h"
#include "bsp.h"
#include "export.h"
#include "index.h"
#include "filter.h"

typedef bsp_node_t* (*bsp_binary_op)(bsp_node_t *, bsp_node_t *);

// A generalization of a binary CSG operation being performed on an STL
// in `path1` and `path1` defined by an operation `op` from` bsp.h
// Result is freshly allocated, and needs to be freed with `free_bsp_tree()`
bsp_node_t* bsp_binary_operation(char *path1, char *path2, bsp_binary_op op) {
	stl_object *file1 = NULL;
	bsp_node_t *bsp1 = NULL;

	stl_object *file2 = NULL;
	bsp_node_t *bsp2 = NULL;

	bsp_node_t *result = NULL;

	// Read 1
	file1 = stl_read_file(path1, 1);
	check(file1 != NULL, "Failed to read .stl from '%s'", path1);
	log_info("Loaded file: %s %d facets", path1, file1->facet_count);
	bsp1 = stl_to_bsp(file1);
	check_mem(bsp1);

	// Read 2
	file2 = stl_read_file(path2, 1);
	check(file2 != NULL, "Failed to read .stl from '%s'", path2);
	log_info("Loaded file: %s %d facets", path2, file2->facet_count);
	bsp2 = stl_to_bsp(file2);
	check_mem(bsp2);

	// Operate
	result = op(bsp1, bsp2);

	if(file1 != NULL) stl_free(file1);
	if(file2 != NULL) stl_free(file2);
	if(bsp1 != NULL) free_bsp_tree(bsp1);
	if(bsp2 != NULL) free_bsp_tree(bsp2);
	return result;
error:
	if(file1 != NULL) stl_free(file1);
	if(file2 != NULL) stl_free(file2);
	if(bsp1 != NULL) free_bsp_tree(bsp1);
	if(bsp2 != NULL) free_bsp_tree(bsp2);
	if(result != NULL) free_bsp_tree(result);
	return NULL;
}

// Constructor for commands named after the CSG functions they perform.
// Produces a function named `cmd_<name>(int argc, char **argv) that reads
// two files and an optional output path and calls a matching function
// bsp_<name>(bsp_node_t*,bsp_node_t*) and writes the resulting mesh
// to disk as either `./out.stl` or the value of argv[2]
// Uses the above `bsp_binary_operation(..)` wrapper to do most of the
// heavy lifting.
#define MAKE_CSG_COMMAND(name)                                                        \
int cmd_##name(int argc, char **argv) {                                               \
	bsp_node_t *result = NULL;                                                        \
	stl_object *out = NULL;                                                           \
	char *out_path = "./out.stl";                                                     \
                                                                                      \
	check(argc >= 2, "At least two input files required.");                           \
	if(argc > 2) out_path = argv[2];                                                  \
                                                                                      \
	result = bsp_binary_operation(argv[0], argv[1], bsp_##name);                      \
	out = bsp_to_stl(result);                                                         \
    log_info("Writing output to %s", out_path);                                       \
	check(stl_write_file(out, out_path) == 0, "Failed to write STL to %s", out_path); \
                                                                                      \
	if(result != NULL) free_bsp_tree(result);                                         \
	if(out != NULL) stl_free(out);                                                    \
	return 0;                                                                         \
error:                                                                                \
	if(result != NULL) free_bsp_tree(result);                                         \
	if(out != NULL) stl_free(out);                                                    \
	return -1;                                                                        \
}

// Each MAKE_BSP_COMMAND(name) results in a function named
// cmd_<name>(int argc, argv) which calls bsp_<name>() with
// two trees built from files in argv[0] and argv[1]
MAKE_CSG_COMMAND(intersect);
MAKE_CSG_COMMAND(union);
MAKE_CSG_COMMAND(subtract);

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
