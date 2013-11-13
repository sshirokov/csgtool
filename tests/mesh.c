#include <string.h>
#include "clar.h"
#include "mesh.h"

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

void test_mesh__can_create_default(void) {
	mesh_t *mesh = NEW(mesh_t, "MSH");

	cl_assert_equal_i(strncmp(mesh->type, "MSH", 3), 0);
	cl_assert_equal_i(mesh->poly_count(mesh), 0);
	cl_assert(mesh->to_polygons(mesh) == NULL);

	mesh->destroy(mesh);
}

void test_mesh__new_class_has_new_methods(void) {
	test_mesh_t *mesh = NEW(test_mesh_t, "TMS");

	cl_assert_equal_i(strncmp(mesh->_(type), "TMS", 3), 0);
	cl_assert_equal_i(mesh->_(poly_count)(mesh), 666);
	cl_assert(mesh->_(to_polygons(mesh) == NULL));

	mesh->_(destroy)(mesh);
}
