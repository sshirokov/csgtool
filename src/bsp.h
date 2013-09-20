#include "klist.h"
#include "poly.h"

#ifndef __BSP_H
#define __BSP_H

// This many polygon pointers are allocated
// on the stack during bsp_clip_polygons and bsp_clip_polygon_array
// Only when more than this is requested do we reach
// into the heap for more polygon pointers.
// Setting this to zero disables the optimization.
// Exceptionally large values will limit the recursion
// limit.
#ifndef STATIC_POLY_BUFFER_SIZE
#define STATIC_POLY_BUFFER_SIZE 200
#endif

typedef struct s_bsp_node {
	klist_t(poly) *polygons;
	poly_t *divider;

	struct s_bsp_node *front;
	struct s_bsp_node *back;
} bsp_node_t;

bsp_node_t *alloc_bsp_node(void);
bsp_node_t *clone_bsp_tree(bsp_node_t *tree);

void free_bsp_node(bsp_node_t *node);
void free_bsp_tree(bsp_node_t *tree);

int bsp_subdivide(poly_t *divider, poly_t *poly,
				  poly_t **coplanar_front, int *n_cp_front,
				  poly_t **coplanar_back,  int *n_cp_back,
				  poly_t **front,          int *n_front,
				  poly_t **back,           int *n_back,
				  poly_t **unused,         int *n_unused);

bsp_node_t *bsp_build(bsp_node_t *node, klist_t(poly) *polygons, int copy);
bsp_node_t *bsp_build_array(bsp_node_t *node, poly_t **polygons, size_t n_polys);
klist_t(poly) *bsp_to_polygons(bsp_node_t *tree,  int make_triangles, klist_t(poly) *dst);

bsp_node_t *bsp_invert(bsp_node_t *tree);
bsp_node_t *bsp_clip(bsp_node_t *us, bsp_node_t *them);
klist_t(poly) *bsp_clip_polygons(bsp_node_t *node, klist_t(poly) *polygons, klist_t(poly) *dst);

// CSG Operations
bsp_node_t *bsp_union(bsp_node_t *a, bsp_node_t *b);
bsp_node_t *bsp_subtract(bsp_node_t *a, bsp_node_t *b);
bsp_node_t *bsp_intersect(bsp_node_t *a, bsp_node_t *b);

#endif
