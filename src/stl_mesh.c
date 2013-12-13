#include "stl_mesh.h"

// Mesh type prototype methods
int stl_mesh_init(void *self, void *data) {
	stl_mesh_t *mesh = (stl_mesh_t*)self;
	if(data == NULL) {
		check_mem(mesh->stl = stl_alloc(NULL, 0));
	}
	else {
		mesh->stl = (stl_object*)data;
	}
	return 0;
error:
	return -1;
}

void stl_mesh_destroy(void *self) {
	stl_mesh_t *mesh = (stl_mesh_t*)self;
	stl_free(mesh->stl);
	free(self);
}

int stl_mesh_poly_count(void *self) {
	stl_mesh_t *mesh = (stl_mesh_t*)self;
	return mesh->stl->facet_count;
}

klist_t(poly)* stl_mesh_to_polygons(void *self) {
	stl_mesh_t *mesh = (stl_mesh_t*)self;
	int count = mesh->_(poly_count)(mesh);
	klist_t(poly)* polys = kl_init(poly);

	for(int i = 0; i < count; i++) {
		poly_t *poly = poly_make_triangle(mesh->stl->facets[i].vertices[0],
										  mesh->stl->facets[i].vertices[1],
										  mesh->stl->facets[i].vertices[2]);
		check_mem(poly);
		*kl_pushp(poly, polys) = poly;
	}


	return polys;
error:
	if(polys != NULL) kl_destroy(poly, polys);
	return NULL;
}

// Mesh type definitions
mesh_t stl_mesh_t_Proto = {
	.init = stl_mesh_init,
	.destroy = stl_mesh_destroy,
	.poly_count = stl_mesh_poly_count,
	.to_polygons = stl_mesh_to_polygons
};
