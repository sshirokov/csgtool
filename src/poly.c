#include <assert.h>

#include "poly.h"

poly_t *alloc_poly(void) {
	poly_t *poly = calloc(1, sizeof(poly_t));
	check_mem(poly);
	poly_init(poly);
	return poly;
error:
	return NULL;
}

void free_poly(poly_t *p, int free_self) {
	if(p == NULL) return;
	if(free_self) free(p);
}

poly_t *poly_init(poly_t *poly) {
	poly->normal[0]  = 0.0;
	poly->normal[1]  = 0.0;
	poly->normal[2]  = 0.0;
	poly->w          = 0.0;
	poly->vertex_count = 0;
	return poly;
}

poly_t *clone_poly(poly_t *poly) {
	poly_t *copy = NULL;
	check_mem(copy = alloc_poly());
	memcpy(copy, poly, sizeof(poly_t));
	return copy;
error:
	return NULL;
}

int poly_update(poly_t *poly) {
	check(poly_vertex_count(poly) > 2,
		  "poly_update(Polyon(%p)): has only %d verticies.",
		  poly, poly_vertex_count(poly));

	float3 *a = &poly->vertices[0];
	float3 *b = &poly->vertices[1];
	float3 *c = &poly->vertices[2];

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

int poly_vertex_count(poly_t *poly) {
	return poly->vertex_count;
}

// Add a vertex to the end of the polygon vertex list
int poly_push_vertex(poly_t *poly, float3 v) {
	check(poly->vertex_count < POLY_MAX_VERTS, "Poly(%p) tried to add %d verts.",
		  poly, poly->vertex_count + 1);

	poly->vertices[poly->vertex_count][0] = v[0];
	poly->vertices[poly->vertex_count][1] = v[1];
	poly->vertices[poly->vertex_count][2] = v[2];
	poly->vertex_count++;

	return poly_update(poly);
error:
	return -1;
}

int poly_classify_vertex(poly_t *poly, float3 v) {
	float side = f3_dot(poly->normal, v) - poly->w;
	if(side < -EPSILON) return BACK;
	if(side > EPSILON) return FRONT;
	return COPLANAR;
}

int poly_classify_poly(poly_t *this, poly_t *other) {
	int front, back;

	front = 0;
	back = 0;

	for(int i = 0; i < poly_vertex_count(other); i++) {
		switch(poly_classify_vertex(this, other->vertices[i])) {
		case FRONT:
			front += 1;
			break;
		case BACK:
			back += 1;
			break;
		}
	}
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

	front_back[0].vertices = front;
	front_back[1].vertices = back;

	for(int j = 0; j < 2; j++) {
		poly_update(&front_back[j]);
	}

	return front_back;
error:
	return NULL;
}

poly_t *poly_make_triangle(float3 a, float3 b, float3 c) {
	poly_t *p = NULL;
	float3 *f = NULL;
	check_mem(p = alloc_poly());

	check_mem(f = clone_f3(a));
	*kl_pushp(float3, p->vertices) = f;
	check_mem(f = clone_f3(b));
	*kl_pushp(float3, p->vertices) = f;
	check_mem(f = clone_f3(c));
	*kl_pushp(float3, p->vertices) = f;

	check(poly_update(p) == 0, "Failed to update polygon(%p) from (%f, %f, %f) (%f, %f, %f) (%f, %f, %f)",
		  p, FLOAT3_FORMAT(a), FLOAT3_FORMAT(b), FLOAT3_FORMAT(c));
	return p;
error:
	if(p) free_poly(p, 1);
	return NULL;
}

void _reverse_vertices(kliter_t(float3) *begin, kliter_t(float3) *end, klist_t(float3) *dst) {
	if(begin != end) {
		_reverse_vertices(kl_next(begin), end, dst);
		assert((*kl_pushp(float3, dst) = clone_f3(*kl_val(begin))) != NULL);
	}
}

poly_t *poly_invert(poly_t *poly) {
	f3_scale(&poly->normal, -1.0);
	poly->w *= -1.0;

	klist_t(float3) *r_vertices = kl_init(float3);
	_reverse_vertices(kl_begin(poly->vertices), kl_end(poly->vertices), r_vertices);
	check(r_vertices->size == poly->vertices->size, "wrong number of verticeis: %zd != %zd", r_vertices->size, poly->vertices->size);

	kl_destroy(float3, poly->vertices);
	poly->vertices = r_vertices;

	return poly;
error:
	kl_destroy(float3, r_vertices);
	return NULL;
}
