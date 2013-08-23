#include <stdlib.h>
#include "klist.h"

#include "poly.h"

#include "index.h"

typedef struct s_idx_poly {
	struct s_vertex_node_t *vertices[POLY_MAX_VERTS];
	int vertex_count;

	poly_t *poly;
} idx_poly_t;

#define mp_free(x) free(kl_val(x))
#define mp_free_idx_poly(x) do { \
	free(kl_val(x)->vertices);	 \
	mp_free(x);                  \
} while(0)
KLIST_INIT(idx_poly, struct s_idx_poly *, mp_free_idx_poly)

typedef struct s_vertex_node {
	float3 vertex;

	klist_t(idx_poly) *polygons;

	struct s_vertex_node *lt;
	struct s_vertex_node *gt;
} vertex_node_t;

vertex_node_t *alloc_vertex_node(void) {
	vertex_node_t *node = calloc(1, sizeof(vertex_node_t));
	node->polygons = kl_init(idx_poly);
	return node;
}

vertex_node_t *search_vertex_node(vertex_node_t *tree, float3 v) {
	if(tree == NULL) return NULL;

	switch(f3_cmp(tree->vertex, v)) {
	case -1: return search_vertex_node(tree->lt, v);
	case 1:  return search_vertex_node(tree->gt, v);
	case 0: return tree;
	}
	return NULL;
}

vertex_node_t *insert_vertex_node(vertex_node_t *tree, float3 v) {
	vertex_node_t *node = NULL;
	if(tree == NULL) {
		node = alloc_vertex_node();
		check_mem(node);
		FLOAT3_SET(node->vertex, v);
	}
	else {
		switch(f3_cmp(tree->vertex, v)) {
		case -1: {
			if(tree->lt != NULL) return insert_vertex_node(tree->lt, v);
			tree->lt = alloc_vertex_node();
			FLOAT3_SET(tree->lt->vertex, v);
			node = tree->lt;
			break;
		}
		case 1: {
			if(tree->gt != NULL) return insert_vertex_node(tree->gt, v);
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

void free_vertex_tree(vertex_node_t *tree) {
	if(tree == NULL) return;
	free_vertex_tree(tree->lt);
	free_vertex_tree(tree->gt);
	kl_destroy(idx_poly, tree->polygons);
	free(tree);
}

void *index_create(klist_t(poly) *polygons) {
	vertex_node_t *verts = NULL;

	// Get all the verticies copied into the buffer
	kliter_t(poly) *iter = kl_begin(polygons);
	for(; iter != kl_end(polygons); iter = kl_next(iter)) {
		poly_t *p = kl_val(iter);
		for(int v = 0; v < p->vertex_count; v++) {
			vertex_node_t *vn = search_vertex_node(verts, p->vertices[v]);
			if(vn == NULL) vn = insert_vertex_node(verts, p->vertices[v]);
			if(verts == NULL) verts = vn;
		}
	}

	if(verts) free_vertex_tree(verts);
	return NULL;
}
