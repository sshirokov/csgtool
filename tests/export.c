#include "clar.h"

#include "stl.h"
#include "bsp.h"

stl_object *stl = NULL;
bsp_node_t *bsp_tree = NULL;

void test_export__initialize(void) {
	// I'll make a quad by making a triangle with a missing
	// vertex, then pushing the extra vertex after and recomputing
	float3 quad_verts[] = {{-1.0, 1.0, 0.0},
						  {-1.0, -1.0, 0.0},
						  {1.0, -1.0, 0.0},
						  {1.0, 1.0, 0.0}};
	poly_t *poly = poly_make_triangle(quad_verts[0], quad_verts[1], quad_verts[2]);
	cl_assert_(poly != NULL, "Can't make triangle for test");

	float3 *f3rc = *kl_pushp(float3, poly->vertices) = clone_f3(quad_verts[3]);
	cl_assert_(f3rc != NULL, "Failed to clone vertex into quad");
	poly_update(poly);

	// Build a tree of the quad
	klist_t(poly) *lpoly = kl_init(poly);
	*kl_pushp(poly, lpoly) = poly;
	bsp_tree = bsp_build(NULL, lpoly);
	cl_assert(bsp_tree);
}

void test_export__cleanup(void) {
	if(stl) stl_free(stl);
	// TODO: free_bsp(bsp)
}
