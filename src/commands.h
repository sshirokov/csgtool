#include <stdio.h>

#ifndef __COMMANDS_H
#define __COMMANDS_H

typedef int (*cmd_fun_t)(int argc, char **argv);

typedef struct s_cmd_t {
	char *name;
	char *description;
	cmd_fun_t fun;
} cmd_t;

extern const cmd_t commands[];
cmd_fun_t cmd_find(const char *name);

#endif
