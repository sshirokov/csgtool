#include "index.h"


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
