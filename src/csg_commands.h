#include <string.h>
#include "dbg.h"

#include "commands.h"
#include "stl.h"
#include "bsp.h"
#include "export.h"

#ifndef __CSG_COMMANDS_H
#define __CSG_COMMANDS_H

typedef bsp_node_t* (*bsp_binary_op)(bsp_node_t *, bsp_node_t *);

bsp_node_t* bsp_binary_operation(char *path1, char *path2, bsp_binary_op op);

#define DEF_CSG_COMMAND(name) int cmd_##name(int argc, char **argv)

// Constructor for commands named after the CSG functions they perform.
// Produces a function named `cmd_<name>(int argc, char **argv) that reads
// two files and an optional output path and calls a matching function
// bsp_<name>(bsp_node_t*,bsp_node_t*) and writes the resulting mesh
// to disk as either `./out.stl` or the value of argv[2]
// Uses the above `bsp_binary_operation(..)` wrapper to do most of the
// heavy lifting.
#define MAKE_CSG_COMMAND(name)                                                        \
DEF_CSG_COMMAND(name) {	                   											  \
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

// Exported commands
DEF_CSG_COMMAND(intersect);
DEF_CSG_COMMAND(union);
DEF_CSG_COMMAND(subtract);

#endif
