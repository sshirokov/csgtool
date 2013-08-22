#include <string.h>
#include "dbg.h"

#include "commands.h"
#include "stl.h"
#include "bsp.h"
#include "export.h"

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

// CLI Command functions
// Each takes argc, counted after the command name
// and argv, with argc elements with the remainder of the
// command line, the commands return status is directly
// returned to the OS.
//
// Commands should be exported in `commands` at the bottom of this file.
int cmd_intersect(int argc, char **argv) {
	bsp_node_t *result = NULL;
	stl_object *out = NULL;
	char *out_path = "/tmp/out.intersect.stl";

	check(argc >= 2, "At least two input files required.");
	if(argc > 2) out_path = argv[2];

	result = bsp_binary_operation(argv[0], argv[1], bsp_intersect);
	out = bsp_to_stl(result);
	check(stl_write_file(out, out_path) == 0, "Failed to write STL to %s", out_path);


	if(result != NULL) free_bsp_tree(result);
	if(out != NULL) stl_free(out);
	return 0;
error:
	if(result != NULL) free_bsp_tree(result);
	if(out != NULL) stl_free(out);
	return -1;
}

int cmd_union(int argc, char **argv) {
	bsp_node_t *result = NULL;
	stl_object *out = NULL;
	char *out_path = "/tmp/out.union.stl";

	check(argc >= 2, "At least two input files required.");
	if(argc > 2) out_path = argv[2];

	result = bsp_binary_operation(argv[0], argv[1], bsp_union);
	out = bsp_to_stl(result);
	check(stl_write_file(out, out_path) == 0, "Failed to write STL to %s", out_path);


	if(result != NULL) free_bsp_tree(result);
	if(out != NULL) stl_free(out);
	return 0;
error:
	if(result != NULL) free_bsp_tree(result);
	if(out != NULL) stl_free(out);
	return -1;
}

int cmd_subtract(int argc, char **argv) {
	bsp_node_t *result = NULL;
	stl_object *out = NULL;
	char *out_path = "/tmp/out.subtract.stl";

	check(argc >= 2, "At least two input files required.");
	if(argc > 2) out_path = argv[2];

	result = bsp_binary_operation(argv[0], argv[1], bsp_subtract);
	out = bsp_to_stl(result);
	check(stl_write_file(out, out_path) == 0, "Failed to write STL to %s", out_path);


	if(result != NULL) free_bsp_tree(result);
	if(out != NULL) stl_free(out);
	return 0;
error:
	if(result != NULL) free_bsp_tree(result);
	if(out != NULL) stl_free(out);
	return -1;
}


// Available commands
const cmd_t commands[] = {
	{"intersect", "Intersect two geometries", cmd_intersect},
	{"subtract",  "Subtract two geometries",  cmd_subtract},
	{"union",     "Union two geometries",     cmd_union},
	{NULL, NULL, NULL}
};

// Search for a command by name.
cmd_fun_t cmd_find(const char *name) {
	for(cmd_t *c = (cmd_t*)commands; c->name != NULL; c++) {
		if(0 == strcmp(c->name, name)) return c->fun;
	}
	return NULL;
}
