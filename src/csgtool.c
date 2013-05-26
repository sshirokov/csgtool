#include <stdio.h>
#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "poly.h"

typedef struct s_bsp_node {
	klist_t(poly) *polygons;
	struct s_bsp_node *divider;

	struct s_bsp_node *front;
	struct s_bsp_node *back;
} bsp_node_t;

bsp_node_t *alloc_bsp_node(void) {
	bsp_node_t *node = NULL;
	check_mem(node = calloc(1, sizeof(bsp_node_t)));
	return node;
error:
	return NULL;
}

void free_bsp_node(bsp_node_t *node) {
	kl_destroy(poly, node->polygons);
	free_bsp_node(node->front);
	free_bsp_node(node->back);
}


int main(int argc, char **argv) {
	check(argc >= 2, "Need a filename");
	char *file = argv[1];
	stl_object *file_stl = stl_read_file(file, 0);
	klist_t(poly) *polygons = kl_init(poly);

	check(file_stl != NULL, "Failed to read stl from '%s'", file);
	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);

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

		printf("Adding %d/%d\r", i+1, file_stl->facet_count);
	}
	putchar('\n');

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
