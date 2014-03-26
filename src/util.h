#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"

#ifndef __UTIL_H
#define __UTIL_H

char *read_line(FILE *f, bool downcase, bool trim);

#endif
