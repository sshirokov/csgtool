#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "dbg.h"

#ifndef __UTIL_H
#define __UTIL_H

// Fatal memory check
#define assert_mem(A) assert("Out of memory." && ((A) != NULL))


char *str_dup(char *str);

char *str_ltrim(char *str, bool copy);
char *str_rtrim(char *str, bool copy);
char *str_trim(char *str, bool copy);

/*
 * Allocate, read, and return a '\0' terminated string from `fd'.
 * Return NULL in caes of error
 */
char *read_line(FILE *f, bool downcase, bool trim);

// Call `read_line` until we don't get blank lines
// Gets the "next" non-blank line
char *next_line(FILE *f, bool downcase, bool trim);

#endif
