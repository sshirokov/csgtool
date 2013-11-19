#include "clar.h"

#include "mesh.h"
#include "reader.h"

char path_to_cube[] = CLAR_FIXTURE_PATH "cube.stl";

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
