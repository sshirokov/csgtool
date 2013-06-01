#include "klist.h"
#include "poly.h"

#ifndef __BSP_H
#define __BSP_H

typedef struct s_bsp_node {
	klist_t(poly) *polygons;
	poly_t *divider;

	struct s_bsp_node *front;
	struct s_bsp_node *back;
} bsp_node_t;

bsp_node_t *alloc_bsp_node(void);
void free_bsp_node(bsp_node_t *node);

int bsp_subdivide(poly_t *divider, poly_t *poly,
				  klist_t(poly) *coplanar_front, klist_t(poly) *coplanar_back,
				  klist_t(poly) *front, klist_t(poly) *back);

bsp_node_t *bsp_build(bsp_node_t *node, klist_t(poly) *polygons);
klist_t(poly) *bsp_to_polygons(bsp_node_t *tree, klist_t(poly) *dst);

#endif
