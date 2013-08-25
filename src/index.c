#include <assert.h>
#include "index.h"

// Indexed polygon methods
idx_poly_t *alloc_idx_poly(poly_t *poly) {
	idx_poly_t *p = NULL;
	check_mem(p = malloc(sizeof(idx_poly_t)));
	p->vertex_count = 0;
	p->poly = poly;

	return p;
error:
	return NULL;
}

void free_idx_poly(idx_poly_t *p) {
	free(p);
}

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

// Edge Tree API
edge_t *alloc_edge(void) {
	edge_t *edge = malloc(sizeof(edge_t));
	check_mem(edge);

	edge->a = edge->b = NULL;
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
	if(tree->lt != NULL) free_edge_tree(tree->lt);
	if(tree->gt != NULL) free_edge_tree(tree->gt);
	free_edge(tree);
}

float3 *edge_middle(edge_t *node, float3 *result) {
	f3_mid(result, node->a->vertex, node->b->vertex);
	return result;
}

edge_t *edge_tree_search_mid(edge_t *tree, float3 mid) {
	if(tree == NULL) return NULL;
	float3 edge_mid = FLOAT3_INIT;
	edge_middle(tree, &edge_mid);

	switch(f3_cmp(edge_mid, mid)) {
	case -1: return edge_tree_search_mid(tree->lt, mid);
	case 1: return edge_tree_search_mid(tree->gt, mid);
	case 0: return tree;
	}
	return NULL;
}

edge_t *edge_tree_search(edge_t *tree, float3 a, float3 b) {
	float3 ab_mid = FLOAT3_INIT;
	f3_mid(&ab_mid, a, b);
	return edge_tree_search_mid(tree, ab_mid);
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

int edge_node_update_verts(edge_t *tree, vertex_node_t *a, vertex_node_t *b) {
	int cmp = f3_cmp(a->vertex, b->vertex);
	check(cmp != 0, "Vertex %p and %p are the same, no edge is formed.", a, b);

	if(cmp < 0) {
		tree->a = a;
		tree->b = b;
	}
	else {
		tree->a = a;
		tree->b = b;
	}

	return 0;
error:
	return -1;
}

edge_t *edge_tree_insert(edge_t *tree, vertex_node_t *a, vertex_node_t *b) {
	int rc = -1;
	edge_t *node = NULL;
	if(tree == NULL) {
		node = alloc_edge();
		check_mem(node);
		node->a = a;
		node->b = b;
	}
	else {
		float3 tree_mid = FLOAT3_INIT;
		float3 ab_mid = FLOAT3_INIT;
		edge_middle(tree, &tree_mid);
		f3_mid(&ab_mid, a->vertex, b->vertex);
		switch(f3_cmp(tree_mid, ab_mid)) {
		case -1: {
			if(tree->lt != NULL) return edge_tree_insert(tree->lt, a, b);
			check_mem(node = alloc_edge());
			rc = edge_node_update_verts(node, a, b);
			check(rc == 0, "Failed to update node %p", node);
			tree->lt = node;
			break;
		}
		case 1: {
			if(tree->gt != NULL) return edge_tree_insert(tree->gt, a, b);
			check_mem(node = alloc_edge());
			rc = edge_node_update_verts(node, a, b);
			check(rc == 0, "Failed to update node %p", node);
			tree->gt = node;
			break;
		}
		default: {
			log_warn("Attempting to insert duplicate edge (%f, %f, %f)-(%f, %f, %f)",
					 FLOAT3_FORMAT(a->vertex), FLOAT3_FORMAT(b->vertex));
			assert(0); // TODO: Handle this case, which shouldn't occur from BSP models
			node = tree;
			break;
		}
		}
	}
	return node;
error:
	if(node != NULL) free_edge(node);
	return NULL;
}

//Overall Mesh index API
mesh_index_t *mesh_index_init(mesh_index_t *idx, klist_t(poly) *polygons) {
	kliter_t(poly) *iter = kl_begin(polygons);
	idx_poly_t *idx_poly = NULL;
	for(; iter != kl_end(polygons); iter = kl_next(iter)) {
		idx_poly = alloc_idx_poly(kl_val(iter));
		check_mem(idx_poly);

		// Store the vertexes in the index vertex tree and update the indexed
		// polygon with the vertex pointers
		for(int v = 0; v < idx_poly->poly->vertex_count; v++) {
			vertex_node_t *vn = vertex_tree_search(idx->vertex_tree, idx_poly->poly->vertices[v]);
			if(vn == NULL) vn = vertex_tree_insert(idx->vertex_tree, idx_poly->poly->vertices[v]);
			if(idx->vertex_tree == NULL) idx->vertex_tree = vn;
			check_mem(vn);
			idx_poly->vertices[idx_poly->vertex_count++] = vn;
			*kl_pushp(idx_poly, vn->polygons) = idx_poly;
		}

		// Store each edge in the edge tree
		for(int i = 0, i2 = 1; i < idx_poly->vertex_count; ++i, i2 = (i + 1) % idx_poly->vertex_count) {
			vertex_node_t *v1 = idx_poly->vertices[i];
			vertex_node_t *v2 = idx_poly->vertices[i2];

			edge_t *edge = edge_tree_search(idx->edge_tree, v1->vertex, v2->vertex);
			if(edge == NULL) edge = edge_tree_insert(idx->edge_tree, v1, v2);
			if(idx->edge_tree == NULL) idx->edge_tree = edge;
			check_mem(edge);
			*kl_pushp(idx_poly, edge->polygons) = idx_poly;
		}

		// Store the indexed polygon in the array
		*kl_pushp(idx_poly, idx->polygons) = idx_poly;
		idx_poly = NULL; // Value is checked for a leak in the error: label
	}
	return idx;
error:
	if(idx_poly) free_idx_poly(idx_poly);
	return NULL;
}

mesh_index_t *alloc_mesh_index(klist_t(poly) *polygons) {
	mesh_index_t *idx = malloc(sizeof(mesh_index_t));
	idx->edge_tree = NULL;
	idx->vertex_tree = NULL;
	idx->polygons = kl_init(idx_poly);
	if(polygons != NULL) {
		check(mesh_index_init(idx, polygons) != NULL, "Failed to initialize mesh index");
	}
	return idx;
error:
	if(idx != NULL) free_mesh_index(idx);
	return NULL;
}

void free_mesh_index(mesh_index_t* idx) {
	if(idx == NULL) return;
	free_vertex_tree(idx->vertex_tree);
	kl_destroy(idx_poly, idx->polygons);
}
