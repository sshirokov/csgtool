#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"

#ifndef __UTIL_H
#define __UTIL_H


char *str_dup(char *str);

char *str_ltrim(char *str, bool copy);
char *str_rtrim(char *str, bool copy);
char *str_trim(char *str, bool copy);

/*
 * Allocate, read, and return a '\0' terminated string from `fd'.
 * Return NULL in caes of error
 */
char *read_line(FILE *f, bool downcase, bool trim);

#endif
