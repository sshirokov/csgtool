#include <stdio.h>

#ifndef __EXPORT_H
#define __EXPORT_H

typedef int (*cmd_fun_t)(int argc, char **argv);

typedef struct s_cmd {
	char *name;
	char *description;
	cmd_fun_t fun;
} cmd_t;

extern const cmd_t commands[];
cmd_fun_t cmd_find(const char *name);

#endif
