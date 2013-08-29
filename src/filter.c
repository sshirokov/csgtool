#include "filter.h"

klist_t(poly) *filter_polys(klist_t(poly) *dst, klist_t(poly) *src, filter_test_t *test) {
	klist_t(poly) *result = NULL;
	if(dst == NULL) result = kl_init(poly);
	else result = dst;
	check_mem(result);


	kliter_t(poly) *iter = kl_begin(src);
	for(; iter != kl_end(src); iter = kl_next(iter)) {
		poly_t *poly = NULL;
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
