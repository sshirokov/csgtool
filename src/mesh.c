#include "dbg.h"

#include "stl.h"
#include "export.h"
#include "mesh.h"

int mesh_init(void *self, void *unused) {
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

int _default_write(void *self, char *path) {
	int rc = -1;
	mesh_t *mesh = (mesh_t*)self;
	stl_object *stl = NULL;
	klist_t(poly) *polys = mesh->to_polygons(mesh);
	check(polys != NULL, "Failed to get polygons from mesh %p", mesh);

	// The default output format is STL
	stl = stl_from_polys(polys);
	check(stl != NULL, "Failed to generate STL object for write.");
	rc = stl_write_file(stl, path);

	if(stl != NULL) stl_free(stl);
	if(polys != NULL) kl_destroy(poly, polys);
	return rc;
error:
	if(stl != NULL) stl_free(stl);
	if(polys != NULL) kl_destroy(poly, polys);
	return -1;
}

void *alloc_mesh(size_t size, mesh_t proto, char type[4], void *data) {
	if(proto.init == NULL) proto.init = mesh_init;
	if(proto.destroy == NULL) proto.destroy = free_mesh;
	if(proto.poly_count == NULL) proto.poly_count = _default_poly_count;
	if(proto.to_polygons == NULL) proto.to_polygons = _default_to_polygons;
	if(proto.write == NULL) proto.write = _default_write;

	mesh_t *m = calloc(1, size);
	*m = proto;
	strncpy(m->type, type, 3);

	check(m->init(m, data) != -1, "Failed to initialize %p(%s, %p)", m, m->type, data);
	return m;
error:
	if(m != NULL) m->destroy(m);
	return NULL;
}
