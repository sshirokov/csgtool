#include "index.h"

// Init, Alloc and Dealloc
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
	free_edge_tree(idx->edge_tree);
	kliter_t(idx_poly) *iter = kl_begin(idx->polygons);
	for(; iter != kl_end(idx->polygons); iter = kl_next(iter)) {
		free_idx_poly(kl_val(iter));
	}
	kl_destroy(idx_poly, idx->polygons);
}

// Access methods
klist_t(edge) *index_find_poly_edges(mesh_index_t *index, idx_poly_t *poly) {
	klist_t(edge) *result = NULL;
	check((result = kl_init(edge)) != NULL,
		  "index_find_poly_edges(%p, %p) Failed to allocate result list.",
		  index, poly);


	for(int i = 0, j = 1; i < poly->vertex_count; j = ((++i) + 1) % poly->vertex_count) {
		edge_t *edge = edge_tree_search(index->edge_tree, poly->vertices[i]->vertex, poly->vertices[j]->vertex);
		if(edge != NULL) {
			*kl_pushp(edge, result) = edge;
		}
		else {
			log_warn("Edge [%p][%d](%f, %f, %f)<->[%p][%d](%f, %f, %f) of %p not found in index.",
					 poly->vertices[i], i, FLOAT3_FORMAT(poly->vertices[i]->vertex),
					 poly->vertices[j], j, FLOAT3_FORMAT(poly->vertices[j]->vertex),
					 poly);
		}
	}

	return result;
error:
	if(result != NULL) kl_destroy(edge, result);
	return NULL;
}

klist_t(idx_poly) *index_find_poly_neighbors(mesh_index_t *index, idx_poly_t *poly) {
	klist_t(idx_poly) *result = NULL;
	klist_t(edge) *edges = NULL;

	check((result = kl_init(idx_poly)) != NULL,
		  "index_find_neighbors(%p, %p) Failed to allocate result list", index, poly);
	check((edges = index_find_poly_edges(index, poly)) != NULL,
		  "index_find_neighbors(%p, %p) Failed to find poly edges", index, poly);

	// Walk every edge and get the polygons that share that edge.
	// Adding evry polygon that isn't `poly` to the result
	// list.
	kliter_t(edge) *iter = kl_begin(edges);
	for(; iter != kl_end(edges); iter = kl_next(iter)) {
		edge_t *edge = kl_val(iter);
		kliter_t(idx_poly) *pIter = kl_begin(edge->polygons);
		for(; pIter != kl_end(edge->polygons); pIter = kl_next(pIter)) {
			idx_poly_t *edge_poly = kl_val(pIter);
			if(edge_poly != poly) {
				*kl_pushp(idx_poly, result) = edge_poly;
			}
		}
	}


	return result;
error:
	if(result != NULL) kl_destroy(idx_poly, result);
	return NULL;
}
