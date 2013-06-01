#include "clar.h"

#include "bsp.h"

bsp_node_t *bsp = NULL;
klist_t(poly) *polygons;

void test_bsp__initialize(void) {
	bsp = alloc_bsp_node();
	cl_assert_(bsp != NULL, "Out of memory");

	poly_t *poly = NULL;
	poly = alloc_poly();
	cl_assert_(poly != NULL, "Out of memory");

	polygons = kl_init(poly);

	float3 *f = NULL;

	// 1st
	f = calloc(1, sizeof(float3));
	(*f)[0] = -0.5;
	(*f)[1] = 0.0;
	(*f)[2] = 0.0;
	*kl_pushp(float3, poly->vertices) = f;

	f = calloc(1, sizeof(float3));
	(*f)[0] = 0.5;
	(*f)[1] = 0.0;
	(*f)[2] = 0.0;
	*kl_pushp(float3, poly->vertices) = f;

	f = calloc(1, sizeof(float3));
	(*f)[0] = 0.0;
	(*f)[1] = 0.5;
	(*f)[2] = 0.0;
	*kl_pushp(float3, poly->vertices) = f;

	poly_update(poly);
	*kl_pushp(poly, polygons) = poly;

	poly = alloc_poly();

	// 2nd
	f = calloc(1, sizeof(float3));
	(*f)[0] = -0.5;
	(*f)[1] = 0.0;
	(*f)[2] = -0.5;
	*kl_pushp(float3, poly->vertices) = f;

	f = calloc(1, sizeof(float3));
	(*f)[0] = 0.5;
	(*f)[1] = 0.0;
	(*f)[2] = -0.5;
	*kl_pushp(float3, poly->vertices) = f;

	f = calloc(1, sizeof(float3));
	(*f)[0] = 0.0;
	(*f)[1] = 0.5;
	(*f)[2] = -0.5;
	*kl_pushp(float3, poly->vertices) = f;

	poly_update(poly);
	*kl_pushp(poly, polygons) = poly;

	cl_assert_(bsp_build(bsp, polygons) != NULL, "Failed to build bsp tree");
}

void test_bsp__cleanup(void) {

}

void test_bsp__root_has_poly(void) {
	cl_assert_equal_i(bsp->polygons->size, 1);
}

void test_bsp__root_has_back_poly(void) {
	cl_assert_equal_i(bsp->back->polygons->size, 1);
}
