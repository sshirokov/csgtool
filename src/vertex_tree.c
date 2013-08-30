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

typedef struct s_vertex_segment_search {
	float3 start;
	float3 end;
	vertex_node_t *results;
} vertex_segment_info_t;

void _store_if_bisect(vertex_node_t *node, vertex_segment_info_t *info) {
	float3 ba_diff = FLOAT3_INIT;
	float3 bn_diff = FLOAT3_INIT;
	float3 ba_normal = FLOAT3_INIT;
	float3 bn_normal = FLOAT3_INIT;

	if((f3_cmp(node->vertex, info->start) == 0) || (f3_cmp(node->vertex, info->end) == 0)) return;

	f3_sub(&ba_diff, info->end, info->start);
	FLOAT3_SET(ba_normal, ba_diff);
	f3_normalize(&ba_normal);

	f3_sub(&bn_diff, node->vertex, info->start);
	FLOAT3_SET(bn_normal, bn_diff);
	f3_normalize(&bn_normal);

	if(f3_cmp(ba_normal, bn_normal) == 0) {
		if(f3_mag2(ba_diff) > f3_mag2(bn_diff)) {
			vertex_node_t *vertex = vertex_tree_search(info->results, node->vertex);
			if(vertex == NULL) vertex = vertex_tree_insert(info->results, node->vertex);
			if(info->results == NULL) info->results = vertex;
		}
	}
}

vertex_node_t *vertex_tree_search_segment(vertex_node_t *tree, float3 start, float3 end) {
	if(tree == NULL) return NULL;
	vertex_segment_info_t info = {
		.start = {FLOAT3_FORMAT(start)},
		.end = {FLOAT3_FORMAT(end)},
		.results = NULL
	};
	vertex_tree_walk(tree, (vertex_tree_visitor)_store_if_bisect, &info);
	return info.results;
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
