#include <stdio.h>
#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "poly.h"

typedef struct s_bsp_node {
	klist_t(poly) *polygons;
	poly_t *divider;

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

void bsp_subdivide(poly_t *divider, poly_t *poly,
				   klist_t(poly) *coplanar_front, klist_t(poly) *coplanar_back,
				   klist_t(poly) *front, klist_t(poly) *back) {
	log_warn("TODO: subdivide @ %p: %p", divider, poly);
}

bsp_node_t *bsp_build(bsp_node_t *node, klist_t(poly) *polygons) {
	if(node->divider == NULL)
		node->divider = clone_poly(kl_val(kl_begin(polygons)));
	klist_t(poly) *front = kl_init(poly);
	klist_t(poly) *back  = kl_init(poly);

	check_mem(node->divider);

	kliter_t(poly) *iter = kl_begin(polygons);
	poly_t *poly = NULL;
	for(; iter != kl_end(polygons); iter = kl_next(iter)) {
		poly = kl_val(iter);
		bsp_subdivide(node->divider, poly, node->polygons, node->polygons, front, back);
	}

	return node;
error:
	kl_destroy(poly, front);
	kl_destroy(poly, back);
	return NULL;
}


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

		printf("Adding %d/%d\r", i+1, file_stl->facet_count);
	}
	putchar('\n');

	check(bsp_build(file_bsp, polygons) != NULL, "Failed to build bsp tree from %zd polygons", polygons->size);

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
