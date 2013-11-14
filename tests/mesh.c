#include <string.h>
#include "clar.h"
#include "mesh.h"
#include "stl.h"

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
char stl_file[] = CLAR_FIXTURE_PATH "cube.stl";
stl_object *stl_file_object = NULL;

void test_mesh__initialize(void) {
	stl_file_object = stl_read_file(stl_file, 1);
	cl_assert(stl_file_object != NULL);
}

void test_mesh__cleanup(void) {
	if(stl_file_object != NULL) {
		stl_free(stl_file_object);
		stl_file_object = NULL;
	}
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
