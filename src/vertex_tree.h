#include <stdio.h>
#include <stdlib.h>
#include "klist.h"

#include "poly.h"
#include "idx_poly.h"

#ifndef __VERTEX_TREE_H
#define __VERTEX_TREE_H

// Vertex Node. Stores vertexes in a tree
// for de-duplication and stores a list of
// polygons using any given vertex.
typedef struct s_vertex_node {
	float3 vertex;

	klist_t(idx_poly) *polygons;

	struct s_vertex_node *lt;
	struct s_vertex_node *gt;
} vertex_node_t;

typedef void (*vertex_tree_visitor)(vertex_node_t *, void*);

vertex_node_t *alloc_vertex_node(void);
void free_vertex_tree(vertex_node_t *tree);

// Vertex tree traverseral and friends
vertex_node_t *vertex_tree_search(vertex_node_t *tree, float3 v);
vertex_node_t *vertex_tree_insert(vertex_node_t *tree, float3 v);

void vertex_tree_walk(vertex_node_t *tree, vertex_tree_visitor visit, void *blob);

int vertex_node_filter_polygon(vertex_node_t *node, idx_poly_t *poly);

// Vertex tree visitors (vertex_tree_visitor above)
void vertex_node_print(vertex_node_t *node, void *stream);
void vertex_node_count(vertex_node_t *node, void *counter);

#endif
