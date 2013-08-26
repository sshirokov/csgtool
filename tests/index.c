#include "clar.h"

#include "stl.h"
#include "export.h"
#include "index.h"

char cube_path[] = CLAR_FIXTURE_PATH "cube.stl";
stl_object *cube_mesh = NULL;
klist_t(poly) *cube_mesh_polys = NULL;
mesh_index_t *idx = NULL;

void test_index__initialize(void) {
	cube_mesh = stl_read_file(cube_path, 1);
	cl_assert_(cube_mesh != NULL, "Failed to read cube");

	cube_mesh_polys = stl_to_polys(cube_mesh);
	cl_assert_(cube_mesh_polys != NULL, "Failed to export mesh as polygon list.");

	idx = alloc_mesh_index(cube_mesh_polys);
	cl_assert_(idx, "Failed to build index");
	cl_assert_(idx->polygons != NULL, "Poly list does not exist");
	cl_assert_equal_i(idx->polygons->size, cube_mesh_polys->size);
}

void test_index__cleanup(void) {
	if(cube_mesh != NULL) stl_free(cube_mesh);
	if(cube_mesh_polys != NULL) kl_destroy(poly, cube_mesh_polys);
	if(idx) free_mesh_index(idx);
	cube_mesh = NULL;
	cube_mesh_polys = NULL;
	idx = NULL;
}

void test_index__reduces_vertex_count(void) {
	cl_assert_(idx->vertex_tree != NULL, "Vertex tree does not exist");

	size_t count = 0;
	vertex_tree_walk(idx->vertex_tree, vertex_node_count, &count);
	// Assuming that there are three vertexes per polygon
	// since that's what the STL spec mandates
	cl_assert_(count < cube_mesh_polys->size * 3, "Duplicate verticies should be removed in the vertex index");
}

void test_index__edge_count(void) {
	cl_assert_(idx->edge_tree != NULL, "Edge tree does not exist.");

	size_t count = 0;
	edge_tree_walk(idx->edge_tree, edge_node_count, &count);
	// A cube expressed as triangles has 18 edges.
	cl_assert_equal_i(count, 18);
}


// Helper for test below
void check_two_polys_on_edge(edge_t *edge, void *ignored) {
	cl_assert_equal_i(edge->polygons->size, 2);
}

void test_index__edges_of_a_cube_all_have_2_neighbors(void) {
	edge_tree_walk(idx->edge_tree, check_two_polys_on_edge, NULL);
}

void test_index__polygon_can_find_edges(void) {
	idx_poly_t *p = kl_val(kl_begin(idx->polygons));

	klist_t(edge) *edges = index_find_poly_edges(idx, p);
	cl_assert_(edges != NULL, "There should be edges defined for every poly in this cube.");
	cl_assert_equal_i(edges->size, 3);
	if(edges != NULL) kl_destroy(edge, edges);
}

void test_index__polygon_can_find_neighbors(void) {
	idx_poly_t *p = kl_val(kl_begin(idx->polygons));

	klist_t(idx_poly) *polys = index_find_poly_neighbors(idx, p);
	cl_assert_(polys != NULL, "There should be neighbors of every poly in this cube.");
	cl_assert_equal_i(polys->size, 3);
	if(polys != NULL) kl_destroy(idx_poly, polys);
}
