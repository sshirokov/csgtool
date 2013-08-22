#include <string.h>
#include "commands.h"

int cmd_intersect(int argc, char **argv) {

	return 0;
}


// Available commands
const cmd_t commands[] = {
	{"intersect", "Intersect two geometries", cmd_intersect},
	{NULL, NULL, NULL}
};

cmd_fun_t cmd_find(const char *name) {
	for(cmd_t *c = (cmd_t*)commands; c->name != NULL; c++) {
		if(0 == strcmp(c->name, name)) return c->fun;
	}
	return NULL;
}
