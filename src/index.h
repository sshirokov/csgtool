#include <stdio.h>
#include <stdlib.h>
#include "klist.h"

#include "poly.h"

#ifndef __INDEX_H
#define __INDEX_H

// Indexed polygon. Stores a 'classic' polygon
// And pointers to vertex nodes in a similar style
// to poly_t's float3's
typedef struct s_idx_poly {
	struct s_vertex_node *vertices[POLY_MAX_VERTS];
	int vertex_count;

	poly_t *poly;
} idx_poly_t;
#define mp_nop(x)
KLIST_INIT(idx_poly, struct s_idx_poly *, mp_nop)

idx_poly_t *alloc_idx_poly(poly_t *poly);
void free_idx_poly(idx_poly_t *p);

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

vertex_node_t *vertex_tree_search(vertex_node_t *tree, float3 v);
vertex_node_t *vertex_tree_insert(vertex_node_t *tree, float3 v);

void vertex_tree_walk(vertex_node_t *tree, vertex_tree_visitor visit, void *blob);

int vertex_node_filter_polygon(vertex_node_t *node, idx_poly_t *poly);

// Vertex tree visitors (vertex_tree_visitor above)
void vertex_node_print(vertex_node_t *node, void *stream);
void vertex_node_count(vertex_node_t *node, void *counter);

// General API
void *index_create(klist_t(poly) *polygons);

#endif
