#include "mesh.h"

#ifndef __READER_H
#define __READER_H

// API
mesh_t* reader_load(char *path);

// Types
typedef int     (reader_predicate_t)(char *path);
typedef mesh_t* (reader_loader_t)(char *path);

typedef struct s_reader_t {
	char *name;
	reader_predicate_t *test;
	reader_loader_t    *load;
} reader_t;

// Exported list of readers
extern const reader_t readers[];

#endif
