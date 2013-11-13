#include <stdlib.h>

#include "poly.h"

#ifndef __MESH_H
#define __MESH_H

typedef struct s_mesh_t {
	char type[4];

	int (*init)(void *self);
	void (*destroy)(void *self);

	int (*poly_count)(void *self);
	klist_t(poly)* (*to_polygons)(void *self);
} mesh_t;

int mesh_init(void *self);
void free_mesh(void *self);

void *alloc_mesh(size_t size, mesh_t proto, char type[4]);

#define NEW(T, N) alloc_mesh(sizeof(T), T##Proto, N)

#endif
