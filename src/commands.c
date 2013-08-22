#include <string.h>
#include "commands.h"

// CLI Command functions
// Each takes argc, counted after the command name
// and argv, with argc elements with the remainder of the
// command line, the commands return status is directly
// returned to the OS.
//
// Commands should be exported in `commands` at the bottom of this file.
int cmd_intersect(int argc, char **argv) {

	return 0;
}


// Available commands
const cmd_t commands[] = {
	{"intersect", "Intersect two geometries", cmd_intersect},
	{NULL, NULL, NULL}
};

// Search for a command by name.
cmd_fun_t cmd_find(const char *name) {
	for(cmd_t *c = (cmd_t*)commands; c->name != NULL; c++) {
		if(0 == strcmp(c->name, name)) return c->fun;
	}
	return NULL;
}
