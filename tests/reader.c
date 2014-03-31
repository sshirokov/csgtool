#include "clar.h"

#include "mesh.h"
#include "reader.h"

char path_to_cube[] = CLAR_FIXTURE_PATH "cube.stl";
char path_to_text_cube[] = CLAR_FIXTURE_PATH "cube.txt.stl";

void test_reader__has_at_least_one(void) {
	cl_assert(readers[0].name != NULL &&
			  readers[0].test != NULL &&
			  readers[0].load != NULL);
}

void test_reader__list_is_terminated(void) {
	int count = 0;
	const reader_t *r = readers;
	for(; r->name != NULL; r++) {
		count++;
	}
	cl_assert(count > 0);
}

void test_reader__can_read_stl(void) {
	mesh_t *stl = reader_load(path_to_cube);
	cl_assert(stl != NULL);

	int poly_count = stl->poly_count(stl);
	cl_assert_((poly_count > 11) && (poly_count < 20), "A cube should have somewhere between 11-20 polys");

	stl->destroy(stl);
}

void test_reader__can_read_text_stl_with_blank_lines(void) {
	mesh_t *stl = reader_load(path_to_text_cube);
	cl_assert(stl != NULL);

	int poly_count = stl->poly_count(stl);
	cl_assert_((poly_count > 11) && (poly_count < 20), "A cube should have somewhere between 11-20 polys");

	stl->destroy(stl);
}
