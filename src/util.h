#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"

#ifndef __UTIL_H
#define __UTIL_H

/*
 * Allocate, read, and return a '\0' terminated string from `fd'.
 * Return NULL in caes of error
 */
char *read_line(FILE *f, bool downcase, bool trim);

#endif
