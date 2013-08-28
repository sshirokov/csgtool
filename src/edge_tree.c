#include <assert.h>

#include "edge_tree.h"

// Edge Tree API
edge_t *alloc_edge(void) {
	edge_t *edge = malloc(sizeof(edge_t));
	check_mem(edge);

	edge->vertex = NULL;
	edge->endpoints = NULL;
	edge->start = NULL;
	edge->lt = edge->gt = NULL;

	edge->polygons = kl_init(idx_poly);
	check_mem(edge->polygons);

	return edge;
error:
	if(edge != NULL) free_edge(edge);
	return NULL;
}

void free_edge_end(edge_t *end) {
	if(end == NULL) return;
	if(end->lt != NULL) free_edge_end(end->lt);
	if(end->gt != NULL) free_edge_end(end->gt);
	if(end->polygons) kl_destroy(idx_poly, end->polygons);
	free(end);
}

void free_edge(edge_t *edge) {
	if(edge == NULL) return;
	free_edge_end(edge->endpoints);
	if(edge->polygons) kl_destroy(idx_poly, edge->polygons);
	free(edge);
}

void free_edge_tree(edge_t *tree) {
	if(tree == NULL) return;
	if(tree->lt != NULL) free_edge_tree(tree->lt);
	if(tree->gt != NULL) free_edge_tree(tree->gt);
	free_edge(tree);
}

void edge_tree_walk_tails(edge_t *start, edge_t *end, edge_tree_visitor visit, void *blob) {
	if((end != NULL) && (end->lt != NULL)) edge_tree_walk_tails(start, end->lt, visit, blob);
	if((end != NULL) && (end->gt != NULL)) edge_tree_walk_tails(start, end->gt, visit, blob);
	if(end != NULL) visit(end, blob);
}

void edge_tree_walk(edge_t *tree, edge_tree_visitor visit, void* blob) {
	if((tree != NULL) && (tree->lt != NULL)) edge_tree_walk(tree->lt, visit, blob);
	if((tree != NULL) && (tree->gt != NULL)) edge_tree_walk(tree->gt, visit, blob);
	edge_tree_walk_tails(tree, tree->endpoints, visit, blob);
}

void edge_node_count(edge_t *node, void *counter) {
	size_t *i = (size_t*)counter;
	if(node != NULL) *i += 1;
}

// Helper to `edge_tree_search' for searching the `endpoints' subtree of the
// edge_t node `tree` for a node where `f3_cmp(tree->vertes->vertex, b) == 0`
edge_t *edge_tree_search_end(edge_t *tree, float3 b) {
	if(tree == NULL) return NULL;

	switch(f3_cmp(tree->vertex->vertex, b)) {
	case -1: return edge_tree_search_end(tree->lt, b);
	case 1: return edge_tree_search_end(tree->gt, b);
	case 0: return tree;
	}
	return NULL;
}

edge_t *edge_tree_search_begin(edge_t *tree, float3 a) {
	if(tree == NULL) return NULL;

	switch(f3_cmp(tree->vertex->vertex, a)) {
	case -1:
		return edge_tree_search_begin(tree->lt, a);
	case 1:
		return edge_tree_search_begin(tree->gt, a);
	case 0:
		return tree;
	}

	return NULL;
}

edge_t *edge_tree_search(edge_t *tree, float3 a, float3 b) {
	if(tree == NULL) return NULL;

	// Ensure we search as A<B
	float3 start, end;
	if(f3_cmp(a, b) < 0) {
		FLOAT3_SET(start, a);
		FLOAT3_SET(end, b);
	}
	else {
		FLOAT3_SET(start, b);
		FLOAT3_SET(end, a);
	}
	edge_t *start_node = edge_tree_search_begin(tree, start);
	if(start_node) return edge_tree_search_end(start_node->endpoints, end);
	return NULL;
}

edge_t *edge_tree_insert_begin(edge_t *tree, vertex_node_t *a) {
	edge_t *edge = NULL;

	// If we're creating a root node, we don't
	// have to do anything other than assignment
	if(tree == NULL) {
		edge = alloc_edge();
		edge->vertex = a;
		return edge;
	}

	switch(f3_cmp(tree->vertex->vertex, a->vertex)) {
	case -1: {
		if(tree->lt != NULL) return edge_tree_insert_begin(tree->lt, a);
		check_mem((edge = alloc_edge()));
		edge->vertex = a;
		tree->lt = edge;
		break;
	}
	case 1: {
		if(tree->gt != NULL) return edge_tree_insert_begin(tree->gt, a);
		check_mem((edge = alloc_edge()));
		edge->vertex = a;
		tree->gt = edge;
		break;
	}
	default: {
		log_err("Attempt to insert a duplicate start point into the edge tree.");
		log_err("Existing node %p->vertex(%p) attempted new A: %p", tree, tree->vertex, a);
		edge = tree;
		break;
	}
	}

	return edge;
error:
	return NULL;
}

edge_t *edge_tree_insert_end(edge_t *tree, edge_t *start, vertex_node_t *b) {
	edge_t *edge = NULL;

	if(tree == NULL) {
		edge = alloc_edge();
		edge->vertex = b;
		edge->start = start;
		return edge;
	}

	switch(f3_cmp(tree->vertex->vertex, b->vertex)) {
	case -1: {
		if(tree->lt != NULL) return edge_tree_insert_end(tree->lt, start, b);
		check_mem((edge = alloc_edge()));
		edge->vertex = b;
		edge->start = start;
		tree->lt = edge;
		break;
	}
	case 1: {
		if(tree->gt != NULL) return edge_tree_insert_end(tree->gt, start, b);
		check_mem((edge = alloc_edge()));
		edge->vertex = b;
		edge->start = start;
		tree->gt = edge;
		break;
	}
	default: {
		log_err("Attempt to insert a duplicate end point into the edge tree.");
		log_err("Existing node %p->vertex(%p)(%f, %f, %f) attempted new B: %p(%f, %f, %f",
				tree, tree->vertex, FLOAT3_FORMAT(tree->vertex->vertex), b, FLOAT3_FORMAT(b->vertex));
		edge = tree;
		break;
	}
	}
	return edge;
error:
	return NULL;
}

edge_t *edge_tree_insert(edge_t *tree, vertex_node_t *a, vertex_node_t *b) {
	// Ensure we search as A<B
	vertex_node_t *start, *end;
	if(f3_cmp(a->vertex, b->vertex) < 0) {
		start = a;
		end = b;
	}
	else {
		start = b;
		end = a;
	}

	edge_t *end_node = NULL;
	edge_t *start_node = NULL;
	start_node = edge_tree_search_begin(tree, start->vertex);
	if(start_node == NULL) start_node = edge_tree_insert_begin(tree, start);

	end_node = edge_tree_insert_end(start_node->endpoints, start_node, end);
	if(start_node->endpoints == NULL) start_node->endpoints = end_node;

	return end_node;
}
