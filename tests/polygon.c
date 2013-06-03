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
	// Invert the top, which should then consider the bottom
	// in front of it.
	cl_assert_(poly_invert(top) != NULL, "Failed to invert top.");
	cl_assert_equal_i(poly_classify_poly(top, bottom), FRONT);

	// Make sure the vertex order is correct
	poly_update(top);
	cl_assert_equal_i(poly_classify_poly(top, bottom), BACK);
}
