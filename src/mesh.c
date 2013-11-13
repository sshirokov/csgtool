#include "dbg.h"

#include "mesh.h"

int mesh_init(void *self) {
	if(self == NULL) return -1;
	return 0;
}

char* mesh_describe(void *self) {
	mesh_t *mesh = (mesh_t*)self;
	return mesh->type;
}

void free_mesh(void *self) {
	if(self != NULL) free(self);
	return;
}

int _default_poly_count(void *self) {
	return 0;
}

klist_t(poly)* _default_to_polygons(void *self) {
	return NULL;
}

void *alloc_mesh(size_t size, mesh_t proto, char type[4]) {
	if(proto.init == NULL) proto.init = mesh_init;
	if(proto.destroy == NULL) proto.destroy = free_mesh;
	if(proto.poly_count == NULL) proto.poly_count = _default_poly_count;
	if(proto.to_polygons == NULL) proto.to_polygons = _default_to_polygons;

	mesh_t *m = calloc(1, size);
	*m = proto;
	strncpy(m->type, type, 3);

	check(m->init(m) != -1, "Failed to initialize %p(%s)", m, m->type);
	return m;
error:
	if(m != NULL) m->destroy(m);
	return NULL;
}
