#include <stdio.h>
#include <stdlib.h>
#include "klist.h"

#include "poly.h"
#include "idx_poly.h"
#include "vertex_tree.h"
#include "edge_tree.h"

#ifndef __INDEX_H
#define __INDEX_H

// Generalized mesh index
// couples a vertex_tree_t with a polygon list of indexed polygons
// for easier polygon walks without vertex tree walks
typedef struct s_mesh_index {
	vertex_node_t *vertex_tree;
	edge_t *edge_tree;
	klist_t(idx_poly) *polygons;
} mesh_index_t;

mesh_index_t *alloc_mesh_index(klist_t(poly) *polygons);
void free_mesh_index(mesh_index_t *index);

mesh_index_t *mesh_index_init(mesh_index_t *idx, klist_t(poly) *polygons);

#endif
