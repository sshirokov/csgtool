#include "clar.h"

#include "poly.h"

klist_t(poly) *faces = NULL;
poly_t *top = NULL;
poly_t *bottom = NULL;

void test_polygon__initialize(void) {
	faces = kl_init(poly);

	// Set up two triangles, one on top of each other
	// stored in list `faces` for easy cleanup
	// and refferenced in `top` and `bottom`
	// used for inversion testing.

	float3 fs[] = {{-0.5, 0.0, 0.0},
				   {0.5, 0.0, 0.0},
				   {0.0, 0.5, 0.0}};
	bottom = poly_make_triangle(fs[0], fs[1], fs[2]);
	cl_assert_(bottom != NULL, "Out of memory");
	*kl_pushp(poly, faces) = bottom;

	for(int i = 0; i < 3; i++) {
		fs[i][2] += 1.0;
	}
	top = poly_make_triangle(fs[0], fs[1], fs[2]);
	cl_assert_(top != NULL, "Out of memory");
	*kl_pushp(poly, faces) = top;

	cl_assert_equal_i(poly_classify_poly(top, bottom), BACK);
	cl_assert_equal_i(poly_classify_poly(bottom, top), FRONT);
}

void test_polygon__cleanup(void) {
	if(faces) kl_destroy(poly, faces);
}

void test_polygon__inverting_top_makes_bottom_front(void) {
	cl_assert_equal_i(poly_classify_poly(top, bottom), BACK);
	poly_t *p = poly_invert(top);
	cl_assert_(p == top, "poly_invert() should return identity");

	cl_assert_equal_i(poly_classify_poly(top, bottom), FRONT);
	poly_update(top);
	cl_assert_equal_i(poly_classify_poly(top, bottom), FRONT);
}

void test_polygon__add_more_than_max_polys(void) {
	poly_t *p = alloc_poly();
	cl_assert(p != NULL);

	float3 point = FLOAT3_INIT;
	float f = 1.0; // y-growth scale
	for(int i = 0; i < POLY_MAX_VERTS * 2; i++) {
		// Might as well build a legitimate polygon
		f3X(point) += 1.0;
		// Scale the growth of Y slowly down to create
		// a sloped curve rather than a staight line.
		f3Y(point) += 1.0 * (f * 0.8);
		poly_push_vertex(p, point);
	}

	cl_assert_(poly_vertex_count(p) > POLY_MAX_VERTS, "Polygon should have more vertex than the static maximum.");
	cl_assert_(poly_vertex_max(p) > POLY_MAX_VERTS, "The maximum verts in the polygon should exceed the static maximum.");

	float3 *last = &p->vertices[poly_vertex_count(p) - 1];

	cl_assert_(f3X(point) == f3X(*last), "The last point in the poly should be the last point we pushed.");

	if(p != NULL) free_poly(p, 1);
}
