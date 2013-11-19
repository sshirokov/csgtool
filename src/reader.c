#include "stl.h"
#include "reader.h"

// Wrappers
int _stl_predicate(char *path) {
	log_err("TODO: Actually detect.");
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
