#include "clar.h"

#include "bsp.h"

bsp_node_t *bsp = NULL;
klist_t(poly) *polygons;

void test_bsp__initialize(void) {
	bsp = alloc_bsp_node();
	poly_t *poly = NULL;
	polygons = kl_init(poly);

	float3 fs[] = {{-0.5, 0.0, 0.0},
				   {0.5, 0.0, 0.0},
				   {0.0, 0.5, 0.0}};
	poly = poly_make_triangle(fs[0], fs[1], fs[2]);
	cl_assert_(poly != NULL, "Out of memory");
	*kl_pushp(poly, polygons) = poly;

	float3 fs2[] = {{-0.5, 0.0, -0.5},
				   {0.5, 0.0, -0.5},
				   {0.0, 0.5, -0.5}};
	poly = poly_make_triangle(fs2[0], fs2[1], fs2[2]);
	cl_assert_(poly != NULL, "Out of memory");
	*kl_pushp(poly, polygons) = poly;

	cl_assert_(bsp != NULL, "Out of memory");
	cl_assert_(bsp_build(bsp, polygons) != NULL, "Failed to build bsp tree");
}

void test_bsp__cleanup(void) {
	// TODO: free_bsp() is a segfault, let's not do that at all
}

void test_bsp__root_has_poly(void) {
	cl_assert_equal_i(bsp->polygons->size, 1);
}

void test_bsp__root_has_back_poly(void) {
	cl_assert_equal_i(bsp->back->polygons->size, 1);
}
