#include <string.h>
#include "clar.h"
#include "mesh.h"
#include "stl.h"
#include "reader.h"
#include "bsp.h"
#include "export.h"
#include "bsp_mesh.h"
#include "stl_mesh.h"

// Test types
mesh_t mesh_t_Proto = {};

typedef struct s_test_mesh_t {
	mesh_t proto;
} test_mesh_t;

int always_666(void *self) {
	return 666;
}

mesh_t test_mesh_t_Proto = {
	.poly_count = always_666
};


// Test data
char tmp_out_file[] = CLAR_FIXTURE_PATH "tmp.stl";
char stl_file[] = CLAR_FIXTURE_PATH "cube.stl";
stl_object *stl_file_object = NULL;


void test_mesh__initialize(void) {
	stl_file_object = stl_read_file(stl_file, 1);
	cl_assert(stl_file_object != NULL);

	// Make sure that we don't test stale data
	unlink(tmp_out_file);
}

void test_mesh__cleanup(void) {
	if(stl_file_object != NULL) {
		stl_free(stl_file_object);
		stl_file_object = NULL;
	}

	// Try to not leave behind stale data
	unlink(tmp_out_file);
}

void test_mesh__can_read_and_write_zero_poly_mesh(void) {
	char stl_path[] = CLAR_FIXTURE_PATH "zero.stl";
	mesh_t *mesh = mesh_read_file(stl_path);
	cl_assert_(mesh != NULL, "It should be okay to read a zero poly mesh");
	cl_assert_equal_i(mesh->poly_count(mesh), 0);
	cl_assert_equal_i(mesh->write(mesh, tmp_out_file, "STL"), 0);
}

void test_mesh__can_produce_bsp_tree(void) {
	mesh_t *mesh = NEW(stl_mesh_t, "STL", stl_file_object);
	stl_file_object = NULL; // The mesh will free it

	bsp_node_t *bsp = mesh->to_bsp(mesh);
	cl_assert(bsp != NULL);

	klist_t(poly) *list = bsp_to_polygons(bsp, 0, NULL);
	cl_assert(list->size >= mesh->poly_count(mesh));

	kl_destroy(poly, list);
	free_bsp_tree(bsp);
}

void test_mesh__can_create_default(void) {
	mesh_t *mesh = NEW(mesh_t, "MSH", NULL);

	cl_assert_equal_i(strncmp(mesh->type, "MSH", 3), 0);
	cl_assert_equal_i(mesh->poly_count(mesh), 0);
	cl_assert(mesh->to_polygons(mesh) == NULL);

	mesh->destroy(mesh);
}

void test_mesh__new_class_has_new_methods(void) {
	test_mesh_t *mesh = NEW(test_mesh_t, "TMS", NULL);

	cl_assert_equal_i(strncmp(mesh->_(type), "TMS", 3), 0);
	cl_assert_equal_i(mesh->_(poly_count)(mesh), 666);
	cl_assert(mesh->_(to_polygons(mesh) == NULL));

	mesh->_(destroy)(mesh);
}

void test_mesh__stl_mesh_methods_work(void) {
	stl_mesh_t *mesh = NEW(stl_mesh_t, "STL", stl_file_object);
	klist_t(poly) *polys = NULL;

	cl_assert_equal_i(mesh->_(poly_count)(mesh), stl_file_object->facet_count);
	cl_assert((polys = mesh->_(to_polygons)(mesh)) != NULL);

	kl_destroy(poly, polys);

	mesh->_(destroy)(mesh);
	stl_file_object = NULL;
}

void test_mesh__mesh_can_write_a_readable_file(void) {
	int rc = -1;
	mesh_t *read_cube = NULL;
	mesh_t *written_cube = NULL;

	// Make sure we can read the test file
	read_cube = mesh_read_file(stl_file);
	cl_assert(read_cube != NULL);
	cl_assert(read_cube->poly_count(read_cube) > 0);

	// Make sure we can write without apperant error
	rc = read_cube->write(read_cube, tmp_out_file, "STL");
	cl_assert_(rc == 0, "Failed to write mesh_t to file.");

	// Read our output and compare
	written_cube = mesh_read_file(tmp_out_file);
	cl_assert(written_cube != NULL);

	cl_assert_equal_i(read_cube->poly_count(read_cube), written_cube->poly_count(written_cube));

	// Destroy our peices
	read_cube->destroy(read_cube);
	written_cube->destroy(written_cube);
}

void test_mesh__write_refuses_to_write_unknown_type(void) {
	mesh_t *mesh = NEW(stl_mesh_t, "STL", stl_file_object);

	cl_assert(mesh->write(mesh, tmp_out_file, "NO!") != 0);

	mesh->destroy(mesh);
	stl_file_object = NULL;
}

void test_mesh__bsp_backed_mesh_has_polygons(void) {
	bsp_node_t *bsp = NULL;
	mesh_t *mesh = NULL;

	cl_assert((bsp = stl_to_bsp(stl_file_object)) != NULL);
	cl_assert((mesh = NEW(bsp_mesh_t, "BSP", bsp)) != NULL);

	cl_assert(mesh->poly_count(mesh) >= stl_file_object->facet_count);
	mesh->destroy(mesh);
}

void test_mesh__bsp_backed_mesh_can_write(void) {
	int rc = -1;
	bsp_node_t *bsp = NULL;
	mesh_t *mesh = NULL;
	mesh_t *read_mesh = NULL;

	bsp = stl_to_bsp(stl_file_object);
	cl_assert(bsp != NULL);

	mesh = NEW(bsp_mesh_t, "BSP", bsp);
	cl_assert(mesh != NULL);

	rc = mesh->write(mesh, tmp_out_file, "STL");
	cl_assert_equal_i(rc, 0);

	read_mesh = mesh_read_file(tmp_out_file);
	cl_assert(read_mesh != NULL);

	cl_assert(read_mesh->poly_count(read_mesh) >= stl_file_object->facet_count);

	if(mesh != NULL) mesh->destroy(mesh);
	if(read_mesh != NULL) read_mesh->destroy(read_mesh);
}
