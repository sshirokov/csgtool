#include <assert.h>
#include <stdbool.h>

#include "poly.h"
#include "export.h"

poly_t *alloc_poly(void) {
	poly_t *poly = malloc(sizeof(poly_t));
	check_mem(poly);
	poly_init(poly);
	return poly;
error:
	return NULL;
}

void free_poly(poly_t *p, int free_self) {
	if(p == NULL) return;
	if(poly_vertex_dynamic_p(p) == 1) {
		if(p->vertices != NULL) free(p->vertices);
		p->vertices = NULL;
	}
	if(free_self) free(p);
}

poly_t *poly_init(poly_t *poly) {
	poly->vertex_count = 0;
	poly->vertex_max = POLY_MAX_VERTS;
	poly->vertices = poly->_vbuffer;
	return poly;
}

poly_t *clone_poly(poly_t *poly) {
	poly_t *copy = NULL;
	check_mem(copy = alloc_poly());
	memcpy(copy, poly, sizeof(poly_t));

	// Either point the clone at its own copied
	// buffer, or copy over the dynamic vertex buffer
	if(poly_vertex_dynamic_p(poly) == 0) {
		copy->vertices = copy->_vbuffer;
	}
	else {
		// We can lean on the `copy->*` memebers
		// since they would have been memcpy'd over
		copy->vertices = malloc(poly_vertex_max(copy) * sizeof(float3));
		check_mem(copy->vertices);
		memcpy(copy->vertices, poly->vertices, poly_vertex_max(copy) * sizeof(float3));
	}

	return copy;
error:
	if(copy != NULL) free_poly(copy, 1);
	return NULL;
}

int poly_update(poly_t *poly) {
	if(poly_vertex_count(poly) < 3) return -1;

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
}

float poly_triangle_area(poly_t *triangle) {
	if(poly_vertex_count(triangle) != 3) return NAN;

	float3 *a = &triangle->vertices[0];
	float3 *b = &triangle->vertices[1];
	float3 *c = &triangle->vertices[2];

	float3 b_a = FLOAT3_INIT;
	float3 c_a = FLOAT3_INIT;
	f3_sub(&b_a, *b, *a);
	f3_sub(&c_a, *c, *a);

	float3 cross = FLOAT3_INIT;
	f3_cross(&cross, b_a, c_a);

	float cross_mag = f3_magnitude(&cross);
	// TODO: This means a vertex doubled down
	//       in the list, producing a zero-area
	//       segment in the tri-fan.
	//       WHAT DO!? DO CARE!?
	// assert(cross_mag > 0.0);

	return 0.5 * cross_mag;
}

float poly_area(poly_t *poly) {
	float area = 0.0;
	klist_t(poly) *tris = NULL;

	// Before we get into this memory management bullshit
	// let's see if it's a triangle and we can just end it there
	// TODO: Seriously, though, do it


	// TODO: This is bad, and you should feel bad, just iterate the verts
	//       and sum. That's what poly_to_tris does anyway, except with more
	//       allocation and thrashing.
	check(tris = poly_to_tris(NULL, poly),
		  "Failed to get triangles from %p(%d)",
		  poly, poly_vertex_count(poly));

	kliter_t(poly) *iter = NULL;
	for(iter = kl_begin(tris); iter != kl_end(tris); iter = kl_next(iter)) {
		poly_t *triangle = kl_val(iter);
		area += poly_triangle_area(triangle);
	}

	kl_destroy(poly, tris);
	return area;
error:
	if(tris != NULL) kl_destroy(poly, tris);
	return NAN;
}

bool poly_has_area(poly_t *poly) {
	float area = poly_area(poly);
	check_debug(!isnan(area), "Polygon(%p) area is NaN", poly);

	return area > 0.0;
error:
	return false;
}

int poly_vertex_count(poly_t *poly) {
	return poly->vertex_count;
}

int poly_vertex_max(poly_t *poly) {
	return poly->vertex_max;
}

int poly_vertex_available(poly_t *poly) {
	return poly->vertex_max - poly->vertex_count;
}

// Has the vertex buffer been dynamically allocated?
int poly_vertex_dynamic_p(poly_t *poly) {
	return (poly->vertices != poly->_vbuffer) ? 1 : 0;
}

int poly_vertex_expand(poly_t *poly) {
	// Not using realloc because the original buffer may be struct-owned
	int new_size = poly->vertex_max * 2;
	float3 *new_verts = malloc(new_size * sizeof(float3));
	check_mem(new_verts);

	memcpy(new_verts, poly->vertices, poly->vertex_max * sizeof(float3));
	poly->vertex_max = new_size;

	// Free the existing buffer if it's not part of the struct's space
	if(poly_vertex_dynamic_p(poly) == 1) {
		free(poly->vertices);
	}

	// Install the new vertex buffer
	poly->vertices = new_verts;

	return 0;
error:
	if(new_verts != NULL) free(new_verts);
	return -1;
}

// add a vertex to the end of the polygon vertex list
int poly_push_vertex(poly_t *poly, float3 v) {
	if(poly_vertex_available(poly) == 0) {
		poly_vertex_expand(poly);
	}

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
	int count = poly_vertex_count(other);

	front = 0;
	back = 0;

	for(int i = 0; i < count; i++) {
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

int poly_split(poly_t *divider, poly_t *poly, poly_t **front, poly_t **back) {
	// Create polygons if we were not passed allocated ones
	if(*front == NULL) {
		*front = alloc_poly();
	}
	if(*back == NULL) {
		*back = alloc_poly();
	}

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
			poly_push_vertex(*front, v_cur);
		}
		if(c_cur != FRONT) {
			poly_push_vertex(*back, v_cur);
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

			check(poly_push_vertex(*front, mid_f) == 0,
				  "Failed to push midpoint to front poly(%p)", front);
			check(poly_push_vertex(*back, mid_f) == 0,
				  "Failed to push midpoint to back poly(%p):", back);
		}
	}

	return 0;
error:
	return -1;
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
	int last = poly_vertex_count(poly) - 1;
	int first = 0;
	float3 temp = FLOAT3_INIT;
	for(; first < last; first++, last--) {
		temp[0] = poly->vertices[last][0];
		temp[1] = poly->vertices[last][1];
		temp[2] = poly->vertices[last][2];

		poly->vertices[last][0] = poly->vertices[first][0];
		poly->vertices[last][1] = poly->vertices[first][1];
		poly->vertices[last][2] = poly->vertices[first][2];

		poly->vertices[first][0] = temp[0];
		poly->vertices[first][1] = temp[1];
		poly->vertices[first][2] = temp[2];
	}

	return poly;
}
