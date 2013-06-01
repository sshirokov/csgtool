#include <stdio.h>
#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "bsp.h"

int main(int argc, char **argv) {
	char *file = NULL;
	stl_object *file_stl = NULL;
	bsp_node_t *file_bsp = NULL;
	klist_t(poly) *polygons = kl_init(poly);
	check(argc >= 2, "Need a filename");

	file = argv[1];
	file_stl = stl_read_file(file, 0);
	check(file_stl != NULL, "Failed to read stl from '%s'", file);
	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);

	file_bsp = alloc_bsp_node();
	check_mem(file_bsp);

	for(int i = 0; i < file_stl->facet_count; i++) {
		stl_facet *face = &file_stl->facets[i];
		poly_t *poly = NULL;

		check(poly = alloc_poly(), "Failed to allocate polygon %d", i);
		*kl_pushp(poly, polygons) = poly;

		// Copy each vertex, using a fresh pointer
		// and letting the poly deallocator deal with it
		float3 *f = NULL;
		for(int v = 0; v < 3; v++) {
			check_mem(f = malloc(sizeof(float3)));
			memcpy(f, face->vertices[v], sizeof(float3));
			*kl_pushp(float3, poly->vertices) = f;
		}
		poly_update(poly);
	}

	check(bsp_build(file_bsp, polygons) != NULL, "Failed to build bsp tree from %zd polygons", polygons->size);

	klist_t(poly) *bpolys = bsp_to_polygons(file_bsp, NULL);

	log_info("bsp_to_polygons: %zd polys", bpolys->size);

	kl_destroy(poly, bpolys);

	kl_destroy(poly, polygons);
	stl_free(file_stl);
	log_info("Terminating Success");
	return 0;
error:
	kl_destroy(poly, polygons);
	if(file_stl != NULL) stl_free(file_stl);
	log_err("Terminating Failure");
	return -1;
}
