#include "poly.h"

poly_t *alloc_poly(void) {
	poly_t *poly = calloc(1, sizeof(poly_t));
	check_mem(poly);
	poly->vertices = kl_init(float3);
	return poly;
error:
	return NULL;
}

poly_t *clone_poly(poly_t *poly) {
	poly_t *copy = NULL;
	check_mem(copy = alloc_poly());

	// Copy the simple stuff
	copy->w = poly->w;
	memcpy(copy->normal, poly->normal, sizeof(float3));

	// Copy each vertex in poly's list
	kliter_t(float3) *vIter;
	float3 *clone;
	for(vIter = kl_begin(poly->vertices); vIter != kl_end(poly->vertices); vIter = kl_next(vIter)) {
		check_mem(clone = clone_f3(*kl_val(vIter)));
		*kl_pushp(float3, copy->vertices) = clone;
	}

	return copy;
error:
	return NULL;
}

void free_poly(poly_t *p) {
	kl_destroy(float3, p->vertices);
}

int poly_update(poly_t *poly) {
	check(poly->vertices->size > 2,
		  "poly_update(Polyon(%p)): has only %zd verticies.",
		  poly, poly->vertices->size);

	kliter_t(float3) *v_iter = kl_begin(poly->vertices);
	float3 *a = kl_val(v_iter);
	float3 *b = kl_val(v_iter = kl_next(v_iter));
	float3 *c = kl_val(v_iter = kl_next(v_iter));

	float3 b_a;
	float3 c_a;
	f3_sub(&b_a, *b, *a);
	f3_sub(&c_a, *c, *a);

	f3_cross(&poly->normal, b_a, c_a);
	f3_normalize(&poly->normal);

	poly->w = f3_dot(poly->normal, *a);
	return 0;
error:
	return -1;
}

int poly_classify_vertex(poly_t *poly, float3 v) {
	float side = f3_dot(poly->normal, v) - poly->w;
	if(side < -EPSILON) return BACK;
	if(side > EPSILON) return FRONT;
	return COPLANAR;
}
