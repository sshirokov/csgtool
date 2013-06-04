#include "clar.h"

#include "stl.h"
#include "bsp.h"

bsp_node_t *bsp = NULL;
klist_t(poly) *polygons;

char cube_stl_file[] = CLAR_FIXTURE_PATH "cube.stl";
bsp_node_t *cube_bsp = NULL;

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

	stl_object *stl_cube = stl_read_file(cube_stl_file, 1);
	cl_assert(stl_cube != NULL);
	klist_t(poly) *cube_polys = kl_init(poly);
	for(int i = 0; i < stl_cube->facet_count; i++) {
		poly_t *p = poly_make_triangle(stl_cube->facets[i].vertices[0],
									   stl_cube->facets[i].vertices[1],
									   stl_cube->facets[i].vertices[2]);
		cl_assert(p != NULL);
		*kl_pushp(poly, cube_polys) = p;
	}
	cube_bsp = bsp_build(NULL, cube_polys);
	kl_destroy(poly, cube_polys);
	cl_assert(cube_bsp != NULL);
}

void test_bsp__cleanup(void) {
	// TODO: free_bsp() is a segfault, let's not do that at all
}

void test_bsp__cube_can_invert(void) {
	float3 point = {0.0, 0.0, 0.5};
	kliter_t(poly) *iter = NULL;

	// Make sure that all polygons consider the point in the center
	// behind them.
	klist_t(poly) *polys = bsp_to_polygons(cube_bsp, 0, NULL);
	for(iter = kl_begin(polys); iter < kl_end(polys); iter = kl_next(iter)) {
		cl_assert_equal_i(poly_classify_vertex(kl_val(iter), point), BACK);
	}
	kl_destroy(poly, polys);

	// INVERT
	cl_assert(bsp_invert(cube_bsp) == cube_bsp);

	// Repeat the test and expect the center to now be FRONT
	polys = bsp_to_polygons(cube_bsp, 0, NULL);
	for(iter = kl_begin(polys); iter < kl_end(polys); iter = kl_next(iter)) {
		cl_assert_equal_i(poly_classify_vertex(kl_val(iter), point), FRONT);
	}
	kl_destroy(poly, polys);
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
	klist_t(poly) *results = bsp_to_polygons(cube_bsp, 0, NULL);

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

void test_bsp__tree_can_produce_triangles_from_quads(void) {
	klist_t(poly) *quad = kl_init(poly);
	// I'll make a quad by making a triangle with a missing
	// vertex, then pushing the extra vertex after and recomputing
	float3 quad_verts[] = {{-1.0, 1.0, 0.0},
						  {-1.0, -1.0, 0.0},
						  {1.0, -1.0, 0.0},
						  {1.0, 1.0, 0.0}};
	poly_t *poly = poly_make_triangle(quad_verts[0], quad_verts[1], quad_verts[2]);
	cl_assert_(poly != NULL, "Can't make triangle for test");

	poly_push_vertex(poly, quad_verts[3]);

	cl_assert_(poly_vertex_count(poly) == 4, "Failed to add vertex into quad");
	poly_update(poly);

	// Build a tree of the quad
	klist_t(poly) *lpoly = kl_init(poly);
	*kl_pushp(poly, lpoly) = poly;
	bsp_node_t *quad_bsp = bsp_build(NULL, lpoly);
	cl_assert(quad_bsp);

	klist_t(poly) *tris = bsp_to_polygons(quad_bsp, 1, NULL);
	cl_assert_equal_i(tris->size, 2);

	kl_destroy(poly, quad);
	kl_destroy(poly, lpoly);
	if(tris) kl_destroy(poly, tris);
}
