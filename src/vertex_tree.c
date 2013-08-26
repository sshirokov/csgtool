#include "vertex_tree.h"

// Vertex node methods
vertex_node_t *alloc_vertex_node(void) {
	vertex_node_t *node = calloc(1, sizeof(vertex_node_t));
	node->polygons = kl_init(idx_poly);
	return node;
}

void free_vertex_tree(vertex_node_t *tree) {
	if(tree == NULL) return;
	free_vertex_tree(tree->lt);
	free_vertex_tree(tree->gt);
	kliter_t(idx_poly) *iter = kl_begin(tree->polygons);
	idx_poly_t *poly = NULL;
	for(; iter != kl_end(tree->polygons); iter = kl_next(iter)) {
		poly = kl_val(iter);
		for(int v = 0; v < poly->vertex_count; v++) {
			if(poly->vertices[v] != tree) {
				vertex_node_filter_polygon(poly->vertices[v], poly);
			}
		}
		free_idx_poly(poly);
	}
	kl_destroy(idx_poly, tree->polygons);
	free(tree);
}

vertex_node_t *vertex_tree_search(vertex_node_t *tree, float3 v) {
	if(tree == NULL) return NULL;

	switch(f3_cmp(tree->vertex, v)) {
	case -1: return vertex_tree_search(tree->lt, v);
	case 1:  return vertex_tree_search(tree->gt, v);
	case 0: return tree;
	}
	return NULL;
}

vertex_node_t *vertex_tree_insert(vertex_node_t *tree, float3 v) {
	vertex_node_t *node = NULL;
	if(tree == NULL) {
		node = alloc_vertex_node();
		check_mem(node);
		FLOAT3_SET(node->vertex, v);
	}
	else {
		switch(f3_cmp(tree->vertex, v)) {
		case -1: {
			if(tree->lt != NULL) return vertex_tree_insert(tree->lt, v);
			tree->lt = alloc_vertex_node();
			FLOAT3_SET(tree->lt->vertex, v);
			node = tree->lt;
			break;
		}
		case 1: {
			if(tree->gt != NULL) return vertex_tree_insert(tree->gt, v);
			tree->gt = alloc_vertex_node();
			FLOAT3_SET(tree->gt->vertex, v);
			node = tree->gt;
			break;
		}
		default: {
			log_warn("Attempting to insert duplicate vertex (%f, %f, %f) into tree", FLOAT3_FORMAT(v));
			node = tree;
			break;
		}
		}
	}

	return node;
error:
	return NULL;
}

void vertex_tree_walk(vertex_node_t *tree, vertex_tree_visitor visit, void *blob) {
	if((tree != NULL) && (tree->lt != NULL)) vertex_tree_walk(tree->lt, visit, blob);
	if((tree != NULL) && (tree->gt != NULL)) vertex_tree_walk(tree->gt, visit, blob);
	visit(tree, blob);
}

int vertex_node_filter_polygon(vertex_node_t *node, idx_poly_t *poly) {
	klist_t(idx_poly) *new_polygons = kl_init(idx_poly);
	kliter_t(idx_poly) *iter = kl_begin(node->polygons);
	for(; iter != kl_end(node->polygons); iter = kl_next(iter)) {
		if(kl_val(iter) != poly) {
			*kl_pushp(idx_poly, new_polygons) = kl_val(iter);
		}
	}
	kl_destroy(idx_poly, node->polygons);
	node->polygons = new_polygons;

	// TODO: Handle the OOM error that can occur
	return 0;
}

void vertex_node_print(vertex_node_t *node, void *stream) {
	FILE *out = (FILE*)stream;
	if(out == NULL) out = stdout;
	fprintf(out, "Node: %p\n", node);
	if(node == NULL) {
		fprintf(out, "\tNULL NODE\n");
	}
	else {
		fprintf(out, "\tVertex: (%f, %f, %f)\n", FLOAT3_FORMAT(node->vertex));
		fprintf(out, "\tPolys: %zd\n", node->polygons->size);
		fprintf(out, "\tLT tree: %p\n", node->lt);
		fprintf(out, "\tGT tree: %p\n", node->gt);
	}
}

void vertex_node_count(vertex_node_t *node, void *counter) {
	size_t *i = (size_t*)counter;
	if(node != NULL) *i += 1;
}
