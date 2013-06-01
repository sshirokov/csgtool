#include "clar.h"

#include "stl.h"
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

	float3 fs3[] = {{-0.5, 0.0, 0.5},
				   {0.5, 0.0, 0.5},
				   {0.0, 0.5, 0.5}};
	poly = poly_make_triangle(fs3[0], fs3[1], fs3[2]);
	cl_assert_(poly != NULL, "Out of memory");
	*kl_pushp(poly, polygons) = poly;

 	float3 fs4[] = {{0.0, 0.0, -1.0},
					{0.0, 0.0, 1.0},
					{0.0, 1.0, 0.0}};
	poly = poly_make_triangle(fs4[0], fs4[1], fs4[2]);
	cl_assert_(poly != NULL, "Out of memory");
	*kl_pushp(poly, polygons) = poly;

	cl_assert_(bsp != NULL, "Out of memory");
	cl_assert_(bsp_build(bsp, polygons) != NULL, "Failed to build bsp tree");
}

void test_bsp__cleanup(void) {
	// TODO: free_bsp() is a segfault, let's not do that at all
}

void test_bsp__cube_bsp_can_return_poly_list_of_equal_length(void) {
	// We test that when we get a list of polygons from a BSP tree of a cube
	// and assert that we have the same number of polygons as when we started.
	//
	// A cube is nice here because no faces need to be split, so polygon
	// counts before and after remain the same.
	char cube_path[] = CLAR_FIXTURE_PATH "cube.stl";
	stl_object *stl_cube = stl_read_file(cube_path, 0);
	klist_t(poly) *cube_polys = kl_init(poly);
	cl_assert(stl_cube != NULL);
	cl_assert(stl_cube->facet_count > 0);

	for(int i = 0; i < stl_cube->facet_count; i++) {
		stl_facet *face = &stl_cube->facets[i];
		poly_t *poly = poly_make_triangle(face->vertices[0], face->vertices[1], face->vertices[2]);
		*kl_pushp(poly, cube_polys) = poly;
		cl_assert(poly);
	}

	bsp_node_t *cube_bsp = alloc_bsp_node();
	cl_assert(cube_bsp != NULL);
	cl_assert(bsp_build(cube_bsp, cube_polys) == cube_bsp);
	klist_t(poly) *results = bsp_to_polygons(cube_bsp, NULL);

	cl_assert_equal_i(results->size, stl_cube->facet_count);

	if(stl_cube) stl_free(stl_cube);
	if(results) kl_destroy(poly, results);
	kl_destroy(poly, cube_polys);
}

void test_bsp__root_has_poly(void) {
	cl_assert_equal_i(bsp->polygons->size, 1);
}

void test_bsp__root_has_back_poly(void) {
	cl_assert_(bsp->back != NULL, "No back tree.");
	cl_assert_equal_i(bsp->back->polygons->size, 1);

	bsp_node_t *front_tree = bsp->back->front;
	cl_assert_(front_tree != NULL, "No back->front tree.");
	cl_assert_equal_i(front_tree->polygons->size, 1);

	cl_assert_(front_tree->front == NULL, "Too many front trees.");

	bsp_node_t *back_tree = bsp->back->back;
	cl_assert_(back_tree != NULL, "No back->back tree.");
	cl_assert_equal_i(back_tree->polygons->size, 1);

	cl_assert_(back_tree->back == NULL, "Too many back trees.");
}

void test_bsp__root_has_front_poly(void) {
	cl_assert_(bsp->front != NULL, "No front tree.");
	cl_assert_equal_i(bsp->front->polygons->size, 1);

	bsp_node_t *back_tree = bsp->front->back;
	cl_assert_(back_tree != NULL, "No front->back tree.");
	cl_assert_equal_i(back_tree->polygons->size, 1);

	cl_assert_(back_tree->back == NULL, "Too many back trees.");

	bsp_node_t *front_tree = bsp->front->front;
	cl_assert_(front_tree != NULL, "No front->front tree.");
	cl_assert_equal_i(front_tree->polygons->size, 1);

	cl_assert_(front_tree->back == NULL, "Too many front trees.");
}
