#include "clar.h"

#include "poly.h"

klist_t(poly) *faces = NULL;
poly_t *top = NULL;
poly_t *bottom = NULL;
poly_t *square = NULL;
poly_t *quad = NULL;
poly_t *line = NULL;

void test_polygon__initialize(void) {
	faces = kl_init(poly);

	// Set up three triangles, one on top of each other
	// stored in list `faces` for easy cleanup
	// and refferenced in `top` and `bottom`, and `square`
	// used for inversion testing and area checking.
	// And a square for non-triangular area tests and
	float3 square_faces[] = {{0.0, 0.0, 0.0},
							 {1.0, 0.0, 0.0},
							 {1.0, 1.0, 0.0},
							 {0.0, 1.0, 0.0}};
	square = alloc_poly();
	cl_assert(square != NULL);

	for(int i = 0; i < 3; i++) {
		cl_assert_(poly_push_vertex(square, square_faces[i]) == 0,
				   "Failed to add square vertex");
	}
	cl_assert_equal_i(poly_vertex_count(square), 3);
	*kl_pushp(poly, faces) = square;

	// Set up a quad
	quad = alloc_poly();
	cl_assert(quad != NULL);

	for(int i = 0; i < 4; i++) {
		cl_assert_(poly_push_vertex(quad, square_faces[i]) == 0,
				   "Failed to add quad vertex.");
	}
	cl_assert_equal_i(poly_vertex_count(quad), 4);
	*kl_pushp(poly, faces) = quad;

	// Set up an invalid polygon which is just a line
	line = alloc_poly();
	cl_assert(line != NULL);

	cl_assert(poly_push_vertex(line, square_faces[0]) == 0);
	cl_assert(poly_push_vertex(line, square_faces[1]) == 0);
	cl_assert(poly_push_vertex(line, square_faces[0]) == 0);
	*kl_pushp(poly, faces) = line;

	// Set up two triangles, one on top of each other
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
		f3Y(point) += 1.0 * (f *= 0.8);
		poly_push_vertex(p, point);
	}

	cl_assert_(poly_vertex_count(p) > POLY_MAX_VERTS, "Polygon should have more vertex than the static maximum.");
	cl_assert_(poly_vertex_max(p) > POLY_MAX_VERTS, "The maximum verts in the polygon should exceed the static maximum.");

	float3 *last = &p->vertices[poly_vertex_count(p) - 1];

	cl_assert_(f3X(point) == f3X(*last), "The last point in the poly should be the last point we pushed.");

	if(p != NULL) free_poly(p, 1);
}

void test_polygon__triangle_area_is_nan_for_quad(void) {
	cl_assert(isnan(poly_triangle_area(quad)));
	cl_assert(!isnan(poly_triangle_area(square)));
}

void test_polygon__triangle_area_is_correct_for_unit_square_triangle(void) {
	// One half base times height on a unit. Hardcore arithmetic.
	cl_assert(poly_triangle_area(square) == 0.5);
	cl_assert(poly_triangle_area(square) == poly_area(square));
}

void test_polygon__area_of_quad(void) {
	cl_assert(poly_area(quad) == 1.0);
	cl_assert(poly_area(quad) == poly_area(square) * 2);
}

void test_polygon__poly_has_area_works(void) {
	cl_assert(poly_has_area(square));
	cl_assert(!poly_has_area(line));
}

void test_polygon__can_compute_longest_edge(void) {
	float longest2_quad = poly_max_edge_length2(quad);
	float longest2_right = poly_max_edge_length2(square);

	cl_assert_(longest2_quad == 1.0, "Longest squared side of a unit square is a unit");
	cl_assert_(longest2_right == 2.0, "Hyp^2 of right unit triangle is 2");
}

void test_polygon__can_compute_min_edge(void) {
	float shortest2_quad = poly_min_edge_length2(quad);
	float shortest2_right = poly_min_edge_length2(square);

	cl_assert_(shortest2_quad == 1.0, "Shortest squared side of a unit square is a unit");
	cl_assert_(shortest2_right == 1.0, "Hyp^2 of right unit triangle is 2, and sides are 1");
}
