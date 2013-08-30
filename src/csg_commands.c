#include "csg_commands.h"

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

// Each MAKE_BSP_COMMAND(name) results in a function named
// cmd_<name>(int argc, argv) which calls bsp_<name>() with
// two trees built from files in argv[0] and argv[1]
MAKE_CSG_COMMAND(intersect);
MAKE_CSG_COMMAND(union);
MAKE_CSG_COMMAND(subtract);
