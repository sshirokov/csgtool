#include "clar.h"

#include "stl.h"
#include "export.h"
#include "index.h"
#include "filter.h"

char cube_path[] = CLAR_FIXTURE_PATH "cube.stl";
stl_object *cube_mesh = NULL;
klist_t(poly) *cube_mesh_polys = NULL;
mesh_index_t *cube_idx = NULL;

char badsquare_path[] = CLAR_FIXTURE_PATH "badsquare.stl";
stl_object *badsquare_mesh = NULL;
klist_t(poly) *badsquare_mesh_polys = NULL;
mesh_index_t *badsquare_idx = NULL;

void init_test_set(char *path, stl_object **mesh, klist_t(poly) **polys, mesh_index_t **index) {
	*mesh = stl_read_file(path, 1);
	cl_assert_(*mesh != NULL, "Failed to read object");

	*polys = stl_to_polys(*mesh);
	cl_assert_(*polys != NULL, "Failed to export mesh as polygon list.");

	*index = alloc_mesh_index(*polys);
	cl_assert_(*index, "Failed to build index");
	cl_assert_((*index)->polygons != NULL, "Poly list does not exist");
	cl_assert_equal_i((*index)->polygons->size, (*polys)->size);
}

void clean_test_set(stl_object **mesh, klist_t(poly) **polys, mesh_index_t **index) {
	if(*mesh != NULL) stl_free(*mesh);
	if(*polys != NULL) kl_destroy(poly, *polys);
	if(*index) free_mesh_index(*index);
	*mesh = NULL;
	*polys = NULL;
	*index = NULL;
}

// Test suite init/clean
void test_index__initialize(void) {
	init_test_set(cube_path, &cube_mesh, &cube_mesh_polys, &cube_idx);
	init_test_set(badsquare_path, &badsquare_mesh, &badsquare_mesh_polys, &badsquare_idx);
}

void test_index__cleanup(void) {
	clean_test_set(&cube_mesh, &cube_mesh_polys, &cube_idx);
	clean_test_set(&badsquare_mesh, &badsquare_mesh_polys, &badsquare_idx);
}

// Test cases
void test_index__reduces_vertex_count(void) {
	cl_assert_(cube_idx->vertex_tree != NULL, "Vertex tree does not exist");

	size_t count = 0;
	vertex_tree_walk(cube_idx->vertex_tree, vertex_node_count, &count);
	// Assuming that there are three vertexes per polygon
	// since that's what the STL spec mandates
	cl_assert_(count < cube_mesh_polys->size * 3, "Duplicate verticies should be removed in the vertex index");
}

void test_index__edge_count(void) {
	cl_assert_(cube_idx->edge_tree != NULL, "Edge tree does not exist.");

	size_t count = 0;
	edge_tree_walk(cube_idx->edge_tree, edge_node_count, &count);
	// A cube expressed as triangles has 18 edges.
	cl_assert_equal_i(count, 18);
}


// Helper for test below
void check_two_polys_on_edge(edge_t *edge, void *ignored) {
	cl_assert_equal_i(edge->polygons->size, 2);
}

void test_index__edges_of_a_cube_all_have_2_neighbors(void) {
	cl_assert_(cube_idx->edge_tree != NULL, "Edge tree does not exist.");
	edge_tree_walk(cube_idx->edge_tree, check_two_polys_on_edge, NULL);
}

void test_index__polygon_can_find_edges(void) {
	idx_poly_t *p = kl_val(kl_begin(cube_idx->polygons));

	klist_t(edge) *edges = index_find_poly_edges(cube_idx, p);
	cl_assert_(edges != NULL, "There should be edges defined for every poly in this cube.");
	cl_assert_equal_i(edges->size, 3);
	if(edges != NULL) kl_destroy(edge, edges);
}

void test_index__polygon_can_find_neighbors(void) {
	idx_poly_t *p = kl_val(kl_begin(cube_idx->polygons));

	klist_t(idx_poly) *polys = index_find_poly_neighbors(cube_idx, p);
	cl_assert_(polys != NULL, "There should be neighbors of every poly in this cube.");
	cl_assert_equal_i(polys->size, 3);
	if(polys != NULL) kl_destroy(idx_poly, polys);
}

void test_index__can_add_edge_bisectors(void) {
	klist_t(poly) *cube_map = map_polys_with_index(cube_idx, NULL, cube_mesh_polys, map_bisect_edges);
	klist_t(poly) *badsquare_map = map_polys_with_index(badsquare_idx, NULL, badsquare_mesh_polys, map_bisect_edges);

	cl_assert(cube_map != NULL);
	cl_assert(badsquare_map != NULL);

	// Every poly in the cube map should be 3 vertex, because it was proper
	kliter_t(poly) *i = kl_begin(cube_map);
	for(; i != kl_end(cube_map); i = kl_next(i)) {
		cl_assert_equal_i(kl_val(i)->vertex_count, 3);
	}

	int three = 0;
	int more = 0;
	int other = 0;
	for(i = kl_begin(badsquare_map); i != kl_end(badsquare_map); i = kl_next(i)) {
		int count = kl_val(i)->vertex_count;
		if(count == 3) three++;
		else if(count > 3) more++;
		else other++;
	}
	cl_assert_equal_i(other, 0);
	cl_assert_equal_i(three, 2);
	cl_assert_equal_i(more, 1);
}
