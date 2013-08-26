#include "idx_poly.h"

// Indexed polygon methods
idx_poly_t *alloc_idx_poly(poly_t *poly) {
	idx_poly_t *p = NULL;
	check_mem(p = malloc(sizeof(idx_poly_t)));
	p->vertex_count = 0;
	p->poly = poly;

	return p;
error:
	return NULL;
}

void free_idx_poly(idx_poly_t *p) {
	free(p);
}
