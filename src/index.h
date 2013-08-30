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

// Walking
typedef void (index_edge_visitor)(mesh_index_t *idx, edge_t *edge, void *blob);

void index_walk_edges(mesh_index_t *index, index_edge_visitor *visit, void *blob);

// Searching
klist_t(edge) *index_find_poly_edges(mesh_index_t *index, idx_poly_t *poly);
klist_t(idx_poly) *index_find_poly_neighbors(mesh_index_t *index, idx_poly_t *poly);

#endif
