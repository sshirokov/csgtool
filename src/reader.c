#include "stl.h"
#include "reader.h"
#include "stl_mesh.h"

// API
mesh_t* reader_load(char *path) {
	mesh_t *mesh = NULL;

	// Walk the readers list and return the first thing that passes
	// the test and loads a mesh_t != NULL
	const reader_t *r = NULL;
	for(r = readers; r->name != NULL; r++) {
		if(r->test(path) == 1) {
			if((mesh = r->load(path))) {
				break;
			}
		}
	}

	return mesh;
}

// Wrappers
int _stl_predicate(char *path) {
	// TODO: Valdiate the STL file instead of blindly trying a read.
	//       Ideally, try to read a line, and pass if binary, if it's text
	//       pass only if it begins with /^solid/i
	return 1;
}

mesh_t* _stl_reader(char *path) {
	stl_object *stl = NULL;
	mesh_t *mesh = NULL;

	check((stl = stl_read_file(path, 1)) != NULL,
		  "Failed to read '%s' as STL", path);
	check((mesh = NEW(stl_mesh_t, "STL", stl)) != NULL,
		  "Failed to create mesh from STL(%p)", stl);

	return mesh;
error:
	return NULL;
}

// List of readers that should be tried in order
// {NULL, NULL, NULL} terminated.
const reader_t readers[] = {
	{"STL", _stl_predicate, _stl_reader},
	{NULL, NULL, NULL}
};
