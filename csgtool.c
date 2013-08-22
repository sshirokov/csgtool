#include <stdio.h>
#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "bsp.h"
#include "export.h"
#include "commands.h"

int usage(char *argv0) {
	fprintf(stderr, "Usage: %s <command> [command options]\n", argv0);
	fprintf(stderr, "\nAvailable commands:\n");
	for(cmd_t *c = (cmd_t*)commands; c->name != NULL; c++) {
		fprintf(stderr, "\t%s - %s\n", c->name, c->description);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
	return 0;
}

int main(int argc, char **argv) {
	if(argc < 2) return usage(argv[0]);

	cmd_fun_t command = cmd_find(argv[1]);
	check(command != NULL, "No such command '%s'", argv[1]);
	return  command(argc - 2, argv + 2);
error:
	usage(argv[0]);
	return -1;
}
