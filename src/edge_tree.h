#include "vertex_tree.h"
#include "idx_poly.h"

#ifndef __EDGE_TREE_H
#define __EDGE_TREE_H

// Polygon edge, A<->B, A < B as mesured by f3_cmp()
// A is stored in Vertex, and the other end is stored
// in the `endpoints` memeber as a tree based on `vertex`.
// The endpoint stores the list of polygons, the origin
// node does not store data.
typedef struct s_edge {
	vertex_node_t *vertex;
	struct s_edge *endpoints;

	klist_t(idx_poly) *polygons;

	struct s_edge *lt;
	struct s_edge *gt;
} edge_t;

typedef void (*edge_tree_visitor)(edge_t *, void*);

edge_t *alloc_edge(void);
void free_edge(edge_t *edge);
void free_edge_tree(edge_t *tree);

void edge_tree_walk(edge_t *tree, edge_tree_visitor visit, void* blob);
void edge_node_count(edge_t *node, void *counter);

edge_t *edge_tree_search(edge_t *tree, float3 a, float3 b);
edge_t *edge_tree_insert(edge_t *tree, vertex_node_t *a, vertex_node_t *b);

// edge_t list and destructor
#define mp_nop(x)
KLIST_INIT(edge, struct s_edge *, mp_nop)

#endif
