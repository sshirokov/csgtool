#include "clar.h"

#include "stl.h"
#include "bsp.h"
#include "export.h"

char cube_stl_path[] = CLAR_FIXTURE_PATH "cube.stl";
stl_object *cube_stl = NULL;
klist_t(poly) *cube_polys = NULL;
bsp_node_t *cube_tree = NULL;
klist_t(poly) *square_list = NULL;

void test_export__initialize(void) {
	cube_stl = stl_read_file(cube_stl_path, 1);
	cl_assert_(cube_stl != NULL, "Failed to read cube.");
	cl_assert_(cube_stl->facet_count >= 12, "Cube should be >= 12 facets.");

	cube_polys = kl_init(poly);
	for(int i = 0; i < cube_stl->facet_count; i++) {
		poly_t *poly = poly_make_triangle(cube_stl->facets[i].vertices[0],
										  cube_stl->facets[i].vertices[1],
										  cube_stl->facets[i].vertices[2]);
		cl_assert(poly != NULL);
		*kl_pushp(poly, cube_polys) = poly;
	}
	cube_tree = bsp_build(NULL, cube_polys, 1);
	cl_assert(cube_tree);

	// Construct a square like this, in the Z=0 plane
	// (0,0)       (1, 0)
	// A------------B
	// |            |
	// |            |
	// |            |
	// |            |
	// D------------C
	// (0,-1)       (1, -1)
	float3 a = {0.0, 0.0, 0.0};
	float3 b = {0.0, 1.0, 0.0};
	float3 c = {1.0, -1.0, 0.0};
	float3 d = {0.0, -1.0, 0.0};
	poly_t *square = alloc_poly();
	cl_assert(square != NULL);

	poly_push_vertex(square, a);
	poly_push_vertex(square, b);
	poly_push_vertex(square, c);
	poly_push_vertex(square, d);
	cl_assert_equal_i(4, poly_vertex_count(square));

	square_list = kl_init(poly);
	*kl_pushp(poly, square_list) = square;
	cl_assert_equal_i(square_list->size, 1);
}

void test_export__cleanup(void) {
	if(cube_stl) stl_free(cube_stl);
	if(cube_polys) kl_destroy(poly, cube_polys);
	if(square_list != NULL) {
		kl_destroy(poly, square_list);
		square_list = NULL;
	}
	free_bsp_tree(cube_tree);
}

void test_export__polys_to_tris_can_allocate(void) {
	klist_t(poly) *result = polys_to_tris(NULL, cube_polys);

	cl_assert(cube_polys->size > 0);
	cl_assert_equal_i(result->size, cube_polys->size);

	kl_destroy(poly, result);
}

void test_export__polys_to_tris_makes_triangles(void) {
	klist_t(poly) *result = polys_to_tris(NULL, square_list);

	cl_assert(square_list->size > 0);
	cl_assert(result->size != square_list->size);
	cl_assert_equal_i(result->size, 2);

	kl_destroy(poly, result);
}

void test_export__tree_can_export_stl(void) {
	stl_object *result = bsp_to_stl(cube_tree);
	cl_assert(result != NULL);
	cl_assert_(result->facet_count >= cube_polys->size, "There should be at least as many polys in the result");

	if(result) stl_free(result);
}

void test_export__test_can_export_zero_poly_stl(void) {
	bsp_node_t *tree = alloc_bsp_node();
	stl_object *stl  = bsp_to_stl(tree);

	cl_assert(stl);
	cl_assert_equal_i(stl->facet_count, 0);

	if(tree) free_bsp_tree(tree);
	if(stl) stl_free(stl);
}
