#include <assert.h>

#include "edge_tree.h"

// Edge Tree API
edge_t *alloc_edge(void) {
	edge_t *edge = malloc(sizeof(edge_t));
	check_mem(edge);

	edge->vertex = NULL;
	edge->endpoints = NULL;
	edge->lt = edge->gt = NULL;

	edge->polygons = kl_init(idx_poly);
	check_mem(edge->polygons);

	return edge;
error:
	if(edge != NULL) free_edge(edge);
	return NULL;
}

void free_edge(edge_t *edge) {
	if(edge == NULL) return;
	if(edge->polygons) kl_destroy(idx_poly, edge->polygons);
	free(edge);
}

void free_edge_tree(edge_t *tree) {
	if(tree == NULL) return;
	if(tree->lt != NULL) free_edge_tree(tree->lt);
	if(tree->gt != NULL) free_edge_tree(tree->gt);
	free_edge(tree);
}

void edge_tree_walk(edge_t *tree, edge_tree_visitor visit, void* blob) {
	if((tree != NULL) && (tree->lt != NULL)) edge_tree_walk(tree->lt, visit, blob);
	if((tree != NULL) && (tree->gt != NULL)) edge_tree_walk(tree->gt, visit, blob);
	visit(tree, blob);
}

void edge_node_count(edge_t *node, void *counter) {
	size_t *i = (size_t*)counter;
	if(node != NULL) *i += 1;
}

edge_t *edge_tree_search(edge_t *tree, float3 a, float3 b) {
	// TODO:
	return NULL;
}

edge_t *edge_tree_insert(edge_t *tree, vertex_node_t *a, vertex_node_t *b) {
	// TODO:
	return NULL;
}
