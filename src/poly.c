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
	check(poly->vertex_count < POLY_MAX_VERTS, "Poly(%p) tried to add %d verts.", poly, poly->vertex_count + 1);

	// Dat assignment copy
	poly->vertices[poly->vertex_count][0] = v[0];
	poly->vertices[poly->vertex_count][1] = v[1];
	poly->vertices[poly->vertex_count][2] = v[2];

	// Update the poly if we can
	if(++poly->vertex_count > 2) {
		check(poly_update(poly) == 0, "Failed to update polygon during poly_push_vertex(%p)", poly);
	}

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
	poly_t *front, *back;
	check_mem(front_back = calloc(2, sizeof(poly_t)));

	// Aliases for easier access
	front = &front_back[0];
	back =  &front_back[1];

	// Current and next vertex
	float3 v_cur = FLOAT3_INIT;
	float3 v_next = FLOAT3_INIT;
	// Classifications of the above
	int c_cur, c_next;
	// Loop indexes
	int i, j;
	int count = poly_vertex_count(poly);
	for(i = 0; i < count; i++) {
		j = (i + 1) % count;
		for(int k = 0; k < 3; k++) {
			v_cur[k]  = poly->vertices[i][k];
			v_next[k] = poly->vertices[j][k];
		}

		// Classify the first and next vertex
		c_cur  = poly_classify_vertex(divider, v_cur);
		c_next = poly_classify_vertex(divider, v_next);

		if(c_cur != BACK)  {
			poly_push_vertex(front, v_cur);
		}
		if(c_cur != FRONT) {
			poly_push_vertex(back, v_cur);
		}

		// Interpolate a midpoint if we found a spanning edge
		if((c_cur | c_next) == SPANNING) {
			float3 diff = FLOAT3_INIT;
			f3_sub(&diff, v_next, v_cur);

			float t = divider->w;
			t = t - f3_dot(divider->normal, v_cur);
			t = t / f3_dot(divider->normal, diff);

			float3 mid_f = {v_cur[0], v_cur[1], v_cur[2]};
			f3_interpolate(&mid_f, v_cur, v_next, t);

			check(poly_push_vertex(front, mid_f) == 0,
				  "Failed to push midpoint to front poly(%p)", front);
			check(poly_push_vertex(back, mid_f) == 0,
				  "Failed to push midpoint to back poly(%p):", back);
		}
	}

	for(int l = 0; l < 2; l++) {
		poly_update(&front_back[l]);
	}

	return front_back;
error:
	return NULL;
}

poly_t *poly_make_triangle(float3 a, float3 b, float3 c) {
	poly_t *p = NULL;
	check_mem(p = alloc_poly());

	check(poly_push_vertex(p, a) == 0,
		  "Failed to add vertex a to poly(%p): (%f, %f, %f)", p, FLOAT3_FORMAT(a));
	check(poly_push_vertex(p, b) == 0,
		  "Failed to add vertex b to poly(%p): (%f, %f, %f)", p, FLOAT3_FORMAT(b));
	check(poly_push_vertex(p, c) == 0,
		  "Failed to add vertex c to poly(%p): (%f, %f, %f)", p, FLOAT3_FORMAT(c));

	return p;
error:
	if(p) free_poly(p, 1);
	return NULL;
}

poly_t *poly_invert(poly_t *poly) {
	f3_scale(&poly->normal, -1.0);
	poly->w *= -1.0;

	// We walk the list from the back to the midway point
	// and flip the opposite ends to reverse the poly list.
	int last = (poly_vertex_count(poly) - 1) / 2;
	float3 temp = FLOAT3_INIT;
	for(int i = last - 1; i <= 0; i++) {
		temp[0] = poly->vertices[last - i][0];
		temp[1] = poly->vertices[last - i][1];
		temp[2] = poly->vertices[last - i][2];

		poly->vertices[last - i][0] = poly->vertices[i][0];
		poly->vertices[last - i][1] = poly->vertices[i][1];
		poly->vertices[last - i][2] = poly->vertices[i][2];

		poly->vertices[i][0] = temp[0];
		poly->vertices[i][1] = temp[1];
		poly->vertices[i][2] = temp[2];
	}

	return poly;
}
