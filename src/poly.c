#include "poly.h"

poly_t *alloc_poly(void) {
	poly_t *poly = calloc(1, sizeof(poly_t));
	check_mem(poly);
	poly_init(poly);
	return poly;
error:
	return NULL;
}

poly_t *poly_init(poly_t *poly) {
	bzero(poly, sizeof(poly_t));
	poly->vertices = kl_init(float3);
	return poly;
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
	log_info("Side: %f", side);
	if(side < -EPSILON) return BACK;
	if(side > EPSILON) return FRONT;
	return COPLANAR;
}

int poly_classify_poly(poly_t *this, poly_t *other) {
	int front, back;

	front = 0;
	back = 0;

	kliter_t(float3) *vIter = kl_begin(other->vertices);
	for(;vIter != kl_end(other->vertices); vIter = kl_next(vIter)) {
		log_info("classify_poly Loop iteration.");
		switch(poly_classify_vertex(this, *kl_val(vIter))) {
		case FRONT:
			log_info("Incrementing front of %p, now %d", this, front);
			front += 1;
			break;
		case BACK:
			log_info("Incrementing back of %p, now %d", this, back);
			back += 1;
			break;
		default:
			log_info("Not front or back: %d/%d", front, back);
			break;
		}
	}
	log_info("poly(%p, %zd), front %d, back %d", this, this->vertices->size, front, back);
	if(front > 0 && back == 0)  return FRONT;
	if(back > 0 && front == 0)  return BACK;
	if(front == 0 && back == 0) return COPLANAR;
	return SPANNING;
}

poly_t *poly_split(poly_t *divider, poly_t *poly) {
	poly_t *front_back = NULL;
	klist_t(float3) *front = kl_init(float3);
	klist_t(float3) *back  = kl_init(float3);

	check_mem(front_back = calloc(2, sizeof(poly_t)));

	kliter_t(float3) *v_cur = kl_begin(poly->vertices);
	kliter_t(float3) *v_next = v_cur;
	int c_cur, c_next;
	float3 *clone = NULL;

	for(; v_cur != kl_end(poly->vertices); v_cur = kl_next(v_cur)) {
		// Get v_next to be the next vertext, looping to the beginning
		v_next = kl_next(v_next);
		if(v_next == kl_end(poly->vertices))
			v_next = kl_begin(poly->vertices);

		// Classify the first and next vertex
		c_cur  = poly_classify_vertex(divider, *kl_val(v_cur));
		c_next = poly_classify_vertex(divider, *kl_val(v_next));

		if(c_cur != BACK)  {
			clone = clone_f3(*kl_val(v_cur));
			check_mem(clone);
			*kl_pushp(float3, front) = clone;
		}
		if(c_cur != FRONT) {
			clone = clone_f3(*kl_val(v_cur));
			check_mem(clone);
			*kl_pushp(float3, back)  = clone;
		}

		// Interpolate a midpoint if we found a spanning edge
		if((c_cur | c_next) == SPANNING) {
			float3 diff = FLOAT3_INIT;
			f3_sub(&diff, *kl_val(v_next), *kl_val(v_cur));

			float t = divider->w;
			t = t - f3_dot(divider->normal, *kl_val(v_cur));
			t = t / f3_dot(divider->normal, diff);

			float3 mid_f = FLOAT3_INIT;
			memcpy(&mid_f, kl_val(v_cur), sizeof(float3));
			f3_interpolate(&mid_f, *kl_val(v_cur), *kl_val(v_next), t);

			clone = clone_f3(mid_f);
			check_mem(clone);
			*kl_pushp(float3, front) = clone;
			clone = clone_f3(mid_f);
			check_mem(clone);
			*kl_pushp(float3, back)  = clone;
		}
	}

	// Init our front and back polys
	// and destroy their vertex lists
	// which we'll reassign from the duplicated
	// lists here
	for(int i = 0; i < 2; i++) {
		poly_init(&front_back[i]);
		kl_destroy(float3, front_back[i].vertices);
	}

	log_info("Front has %zd verts", front->size);
	log_info("Back has %zd verts", back->size);

	front_back[0].vertices = front;
	front_back[1].vertices = back;

	for(int j = 0; j < 2; j++) {
		poly_update(&front_back[j]);
	}

	return front_back;
error:
	return NULL;
}
