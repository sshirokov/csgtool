#include "clar.h"

#include "vector.h"
#include "poly.h"

poly_t *poly = NULL;
poly_t *poly_clone = NULL;

void test_classify__initialize(void) {
	poly = alloc_poly();
	cl_assert_(poly != NULL, "Out of memory");

	float3 *f = NULL;
	// Build a triangle, facing +z on the x,y axis
	// The floats will be freed by the poly deallocator
	// (-1, -1, 0)
	f = calloc(1, sizeof(float3));
	(*f)[0] = -1.0;
	(*f)[1] = -1.0;
	(*f)[2] = 0.0;
	*kl_pushp(float3, poly->vertices) = f;

	// (1, -1, 0)
	f = calloc(1, sizeof(float3));
	(*f)[0] = 1.0;
	(*f)[1] = -1.0;
	(*f)[2] = 0.0;
	*kl_pushp(float3, poly->vertices) = f;

	// (0, 1, 0)
	f = calloc(1, sizeof(float3));
	(*f)[0] = 0.0;
	(*f)[1] = 1.0;
	(*f)[2] = 0.0;
	*kl_pushp(float3, poly->vertices) = f;

	int rc = poly_update(poly);
	cl_must_pass_(rc, "Failed to update polygon.");

	float total = poly->normal[0] + poly->normal[1] + poly->normal[2];
	cl_assert_(total > 0.0, "Normal should not be null");

	poly_clone = clone_poly(poly);
	cl_assert_(poly_clone != NULL, "Failed to clone polygon.");
}

void test_classify__cleanup(void) {
	if(poly) free_poly(poly);
	if(poly_clone) free_poly(poly_clone);
}

void test_classify__polygon_vertices_coplanar(void) {
	float3 *v = kl_val(kl_begin(poly->vertices));

	int side = poly_classify_vertex(poly, *v);
	cl_assert_equal_i(side, COPLANAR);
}

void test_classify__polygon_self_dupe_coplanar(void) {
	int side = poly_classify_poly(poly, poly_clone);
	cl_assert_equal_i(side, COPLANAR);
}

void test_classify__polygon_tilted_dupe_coplanar(void) {
	int rc = 0;

	poly_t *another = clone_poly(poly);
	(*kl_val(kl_begin(another->vertices)))[2] += 0.6;
	cl_must_pass(poly_update(another));
	poly_t *another_clone = clone_poly(another);
	cl_must_pass(poly_update(another_clone));

	rc = poly_classify_poly(another, another_clone);
	cl_assert_equal_i(rc, COPLANAR);

	if(another) free_poly(another);
	if(another_clone) free_poly(another_clone);
}
