#include <stdio.h>
#include <stdlib.h>
#include "klist.h"

#include "poly.h"


#ifndef __IDX_POLY_H
#define __IDX_POLY_H

// Indexed polygon. Stores a 'classic' polygon
// And pointers to vertex nodes in a similar style
// to poly_t's float3's
typedef struct s_idx_poly {
	struct s_vertex_node *vertices[POLY_MAX_VERTS];
	int vertex_count;

	poly_t *poly;
} idx_poly_t;
#define mp_nop(x)
KLIST_INIT(idx_poly, struct s_idx_poly *, mp_nop)

idx_poly_t *alloc_idx_poly(poly_t *poly);
void free_idx_poly(idx_poly_t *p);


#endif
