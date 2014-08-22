#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

#include "poly.h"
#include "util.h"
#include "export.h"

poly_t *alloc_poly(void) {
	poly_t *poly = malloc(sizeof(poly_t));
	assert_mem(poly);
	poly_init(poly);
	return poly;
}

void free_poly(poly_t *p, int free_self) {
	if(p == NULL) return;
	if(poly_vertex_dynamic_p(p) == 1) {
		if(p->vertices != NULL) free(p->vertices);
		p->vertices = NULL;
	}
	if(free_self) free(p);
}

void poly_print(poly_t *p, FILE *stream) {
	fprintf(stream, "Poly(%p) Verts: %d Area: %f:\n", p, poly_vertex_count(p), poly_area(p));
	for(int i = 0; i < poly_vertex_count(p); i++) {
		fprintf(stream,"\tV[%d]: (%f, %f, %f)\n", i, FLOAT3_FORMAT(p->vertices[i]));
	}
}

void poly_print_with_plane_info(poly_t *p, poly_t *plane, FILE *stream) {
	fprintf(stream, "Poly(%p) w(%f) Verts: %d Area: %f:\n", p, p->w, poly_vertex_count(p), poly_area(p));
	for(int i = 0; i < poly_vertex_count(p); i++) {
		float3 diff = FLOAT3_INIT;
		f3_sub(&diff, p->vertices[i], plane->vertices[0]);
		float distance = f3_dot(plane->normal, diff);
		fprintf(stream,"\tV[%d]: (%f, %f, %f) [%s] - %f from plane\n",
				i, FLOAT3_FORMAT(p->vertices[i]), poly_classify_vertex_string(plane, p->vertices[i]), distance);
	}
}

poly_t *poly_init(poly_t *poly) {
	poly->vertex_count = 0;
	poly->vertex_max = POLY_MAX_VERTS;
	poly->vertices = poly->_vbuffer;
	return poly;
}

poly_t *clone_poly(poly_t *poly) {
	poly_t *copy = alloc_poly();
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
		assert_mem(copy->vertices);
		memcpy(copy->vertices, poly->vertices, poly_vertex_max(copy) * sizeof(float3));
	}

	return copy;
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

// Return two times the area of a triangle.
// Avoids the division in half unless it's required to avoid
// failing `f > 0.0` when area is used as a predicate
float poly_triangle_2area(poly_t *triangle) {
	if(poly_vertex_count(triangle) != 3) return NAN;

	return triangle_2area(
		triangle->vertices[0],
		triangle->vertices[1],
		triangle->vertices[2]
	);
}

// The actual area of a triangle `triangle`
// Works through poly_triangle_2area
float poly_triangle_area(poly_t *triangle) {
	return 0.5 * poly_triangle_2area(triangle);
}

float poly_area(poly_t *poly) {
	return poly_2area(poly) / 2.0;
}

float poly_2area(poly_t *poly) {
	float area2 = 0.0;
	const int vertex_count = poly_vertex_count(poly);

	// Sanity check that we have at least a polygon
	if(vertex_count < 3) return NAN;

	// Before we get into this tesselating bullshit, is this just a triangle?
	if(vertex_count == 3) return poly_triangle_2area(poly);

	// Break the poly into a triangle fan and sum the 2areas of the components
	// Note that i = 2 on first iteration so that `i - 1` is defined and != 0
	// This starts the loop on the first triangle in the poly.
	// Since we're only caring about the magnitude of the cross inside
	// triangle_2area, the vertex order doesn't matter.
	for(int i = 2; i < vertex_count; i++) {
		area2 += triangle_2area(
			poly->vertices[0],     // Root vertex
			poly->vertices[i - 1], // Previous vertex
			poly->vertices[i]      // Current vertex
	    );
	}


	return area2;
}

bool poly_has_area(poly_t *poly) {
	float area = poly_2area(poly);
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
	assert_mem(new_verts);

	memcpy(new_verts, poly->vertices, poly->vertex_max * sizeof(float3));
	poly->vertex_max = new_size;

	// Free the existing buffer if it's not part of the struct's space
	if(poly_vertex_dynamic_p(poly) == 1) {
		free(poly->vertices);
	}

	// Install the new vertex buffer
	poly->vertices = new_verts;

	return 0;
}

// add a vertex to the end of the polygon vertex list, if
// `guard` is true, a check will be performed to reject
// verts that cause 0-length edges to appear.
bool poly_push_vertex_guarded(poly_t *poly, float3 v, bool guard) {
	if(poly_vertex_available(poly) == 0) {
		poly_vertex_expand(poly);
	}

	// We only need to perform zero-length-edge checks if we are
	// actually going to create an edge through this push.
	if(guard && (poly_vertex_count(poly) > 0)) {
		int last_idx = poly_vertex_count(poly) - 1;
		bool duplicate_first = !(f3_distance2(poly->vertices[0], v) > 0.0);
		bool duplicate_last  = !(f3_distance2(poly->vertices[last_idx], v) > 0.0);

		// Fail out the addition if we're adding a duplucate first or last vertex
		// as the new last vert. This would create an edge of length zero.
		if(duplicate_first || duplicate_last) return false;
	}

	// Dat assignment copy
	poly->vertices[poly->vertex_count][0] = v[0];
	poly->vertices[poly->vertex_count][1] = v[1];
	poly->vertices[poly->vertex_count][2] = v[2];

	// Update the poly if we can
	if(++poly->vertex_count > 2) {
		check(poly_update(poly) == 0, "Failed to update polygon during poly_push_vertex(%p)", poly);
	}

	return true;
error:
	return false;
}

// The default interface to pushing a vertex, force the guard to on
bool poly_push_vertex(poly_t *poly, float3 v) {
	return poly_push_vertex_guarded(poly, v, true);
}

// Unsafe poly push, forces the guard off, allowing 0-length edges
// to form. Useful in the `audit` command
bool poly_push_vertex_unsafe(poly_t *poly, float3 v) {
	return poly_push_vertex_guarded(poly, v, false);
}

int poly_classify_vertex(poly_t *poly, float3 v) {
	float side = f3_dot(poly->normal, v) - poly->w;
	if(side < -EPSILON) return BACK;
	if(side > EPSILON) return FRONT;
	return COPLANAR;
}

const char* poly_classify_vertex_string(poly_t *poly, float3 v) {
	const char *classification = "UNKNOWN";
	switch(poly_classify_vertex(poly, v)) {
	case FRONT:
		classification = "FRONT";
		break;
	case BACK:
		classification = "BACK";
		break;
	case COPLANAR:
		classification = "COPLANAR";
	}
	return classification;
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

		// Fill v_cur[..] and v_next[..] with the values of
		// the current (i) and next (j) vertex (x,y,z) data
		// from `poly`
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

			poly_push_vertex(*front, mid_f);
			poly_push_vertex(*back, mid_f);
		}
	}

	// Clear any polygons that are not finished by this point
	if((*front != NULL) && (poly_vertex_count(*front) < 3)) {
		free_poly(*front, true);
		*front = NULL;
	}

	if((*back != NULL) && (poly_vertex_count(*back) < 3)) {
		free_poly(*back, true);
		*back = NULL;
	}

	return 0;
}

poly_t *poly_make_triangle_guarded(float3 a, float3 b, float3 c, bool guard) {
	poly_t *p = alloc_poly();

	check_debug(poly_push_vertex_guarded(p, a, guard),
		  "Failed to add vertex a to poly(%p): (%f, %f, %f)", p, FLOAT3_FORMAT(a));
	check_debug(poly_push_vertex_guarded(p, b, guard),
		  "Failed to add vertex b to poly(%p): (%f, %f, %f)", p, FLOAT3_FORMAT(b));
	check_debug(poly_push_vertex_guarded(p, c, guard),
		  "Failed to add vertex c to poly(%p): (%f, %f, %f)", p, FLOAT3_FORMAT(c));

	return p;
error:
	if(p) free_poly(p, 1);
	return NULL;
}

poly_t *poly_make_triangle(float3 a, float3 b, float3 c) {
	return poly_make_triangle_guarded(a, b, c, true);
}

poly_t *poly_make_triangle_unsafe(float3 a, float3 b, float3 c) {
	return poly_make_triangle_guarded(a, b, c, false);
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

// Compute the length of the lognest edge squared
float poly_max_edge_length2(poly_t *poly) {
	const int count = poly_vertex_count(poly);
	float longest = -INFINITY;

	for(int i = 0; i < count; i++) {
		int j = (i + 1) % count;
		float d2 = f3_distance2(poly->vertices[i], poly->vertices[j]);

		longest = (d2 > longest) ? d2 : longest;
	}

	return longest;
}

float poly_min_edge_length2(poly_t *poly) {
	const int count = poly_vertex_count(poly);
	float min = INFINITY;

	for(int i = 0; i < count; i++) {
		int j = (i + 1) % count;
		float d2 = f3_distance2(poly->vertices[i], poly->vertices[j]);

		min = (d2 < min) ? d2 : min;
	}

	return min;
}

float triangle_2area(float3 a, float3 b, float3 c) {
	float3 b_a = FLOAT3_INIT;
	float3 c_a = FLOAT3_INIT;

	f3_sub(&b_a, b, a);
	f3_sub(&c_a, c, a);

	float3 cross = FLOAT3_INIT;
	f3_cross(&cross, b_a, c_a);

	return f3_magnitude(&cross);
}
