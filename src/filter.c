#include "filter.h"

int filter_test_edge_singularity(poly_t *poly) {
	for(int i = 0, j = 1; i < poly->vertex_count; j = ((++i) + 1) % poly->vertex_count) {
		float3 *edge[2] = {&poly->vertices[i], &poly->vertices[j]};
		if(f3_cmp(*edge[0], *edge[1]) == 0) {
			return 0;
		}
	}
	return 1;
}

klist_t(poly) *filter_polys(klist_t(poly) *dst, klist_t(poly) *src, filter_test_t *test) {
	klist_t(poly) *result = NULL;
	if(dst == NULL) result = kl_init(poly);
	else result = dst;
	check_mem(result);


	kliter_t(poly) *iter = kl_begin(src);
	for(; iter != kl_end(src); iter = kl_next(iter)) {
		poly_t *poly = kl_val(iter);
		if(test(poly) > 0) {
			poly_t *clone = clone_poly(poly);
			check_mem(clone);
			*kl_pushp(poly, result) = clone;
		}
	}

	return result;
error:
	if((result != NULL) && (result != dst)) kl_destroy(poly, result);
	return NULL;
}
