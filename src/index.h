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

// Vertex tree traverseral and friends
vertex_node_t *vertex_tree_search(vertex_node_t *tree, float3 v);
vertex_node_t *vertex_tree_insert(vertex_node_t *tree, float3 v);

void vertex_tree_walk(vertex_node_t *tree, vertex_tree_visitor visit, void *blob);

int vertex_node_filter_polygon(vertex_node_t *node, idx_poly_t *poly);

// Vertex tree visitors (vertex_tree_visitor above)
void vertex_node_print(vertex_node_t *node, void *stream);
void vertex_node_count(vertex_node_t *node, void *counter);

// Polygon edge, a < b
typedef struct s_edge {
	vertex_node_t *a;
	vertex_node_t *b;

	klist_t(idx_poly) *polygons;

	struct s_edge *lt;
	struct s_edge *gt;
} edge_t;

edge_t *alloc_edge(void);
void free_edge(edge_t *edge);

void free_edge_tree(edge_t *tree);

// edge_t list and destructor
#define mp_free_edge(x) free_edge(kl_val(x))
KLIST_INIT(edge, struct s_edge *, mp_free_edge)

// Generalized mesh index
// couples a vertex_tree_t with a polygon list of indexed polygons
// for easier polygon walks without vertex tree walks
typedef struct s_mesh_index {
	vertex_node_t *vertex_tree;
	klist_t(idx_poly) *polygons;
} mesh_index_t;

mesh_index_t *alloc_mesh_index(klist_t(poly) *polygons);
void free_mesh_index(mesh_index_t *index);

mesh_index_t *mesh_index_init(mesh_index_t *idx, klist_t(poly) *polygons);

#endif
