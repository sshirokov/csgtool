#include <stdlib.h>
#include "klist.h"

#include "filter.h"

int filter_test_edge_singularity(poly_t *poly) {
	for(int i = 0, j = 1; i < poly->vertex_count; j = ((++i) + 1) % poly->vertex_count) {
		float3 *edge[2] = {&poly->vertices[i], &poly->vertices[j]};
		if(f3_cmp(*edge[0], *edge[1]) == 0) {
			return 0;
		}
	}
	return 1;
}


typedef  int(qsort_cmp_t)(const void*,const void*);

typedef struct s_f3_mag {
	float3 base;
	float3 v;
} f3_mag_t;

typedef struct s_f3_mag_buffer {
	float3 base;
	f3_mag_t *buffer;
	size_t n;
} f3_mag_buffer_t;

int cmp_f3_mag2(f3_mag_t *a, f3_mag_t *b) {
	float3 diffA = FLOAT3_INIT;
	float3 diffB = FLOAT3_INIT;
	f3_sub(&diffA, a->base, a->v);
	f3_sub(&diffB, b->base, b->v);

	float magA = f3_mag2(diffA);
	float magB = f3_mag2(diffB);

	if(magA < magB) return -1;
	if(magA > magB) return 1;
	return 0;
}

void add_to_f3_mag_buff(vertex_node_t *node, f3_mag_buffer_t *buffer) {
    // WARNING: MULTIPLE EVALUATION INSIDE FLOAT3_SET!
	int i = buffer->n++;
	FLOAT3_SET(buffer->buffer[i].v, node->vertex);
}

poly_t *poly_bisect_edges(poly_t *poly, mesh_index_t *index) {
	poly_t *new = alloc_poly();
	check_mem(new);
	for(int i = 0, j = 1; i < poly->vertex_count; j = ((++i) + 1) % poly->vertex_count) {
		vertex_node_t *verts = NULL;
		size_t count = 0;
		verts = vertex_tree_search_segment(index->vertex_tree, &count, poly->vertices[i], poly->vertices[j]);
		if(count > 0) {
			// Since we have bisecting verts, we will sort them into a list
			// by magnitude then add any vertexes after the current into the list
			// of the new polygon.
			check_mem(verts);
			f3_mag_buffer_t bisects = {
				.base = {FLOAT3_FORMAT(poly->vertices[j])},
				.buffer = calloc(count, sizeof(f3_mag_t)),
				.n = 0
			};
			check_mem(bisects.buffer);
			vertex_tree_walk(verts, (vertex_tree_visitor)add_to_f3_mag_buff, &bisects);

			qsort(bisects.buffer, bisects.n, sizeof(f3_mag_t), (qsort_cmp_t*)cmp_f3_mag2);

			poly_push_vertex(new, poly->vertices[i]);
			for(int k = 0; k < bisects.n; k++) {
				poly_push_vertex(new, bisects.buffer[k].v);
			}
			free(bisects.buffer);
		}
		else {
			poly_push_vertex(new, poly->vertices[i]);
		}
		free_vertex_tree(verts);
	}

	return new;
error:
	if(new != NULL) free_poly(new, 1);
	return NULL;
}

void map_bisect_edges(klist_t(poly) *dst, mesh_index_t *index, poly_t *poly) {
	// TODO: Memory can run out in here, should probably return an error and handle it
	poly_t *new = poly_bisect_edges(poly, index);
	*kl_pushp(poly, dst) = new;
}

klist_t(poly) *filter_polys(klist_t(poly) *dst, klist_t(poly) *src, filter_test_t *test) {
	klist_t(poly) *result = NULL;
	if(dst == NULL) result = kl_init(poly);
	else result = dst;
	check_mem(result);


	kliter_t(poly) *iter = kl_begin(src);
	for(; iter != kl_end(src); iter = kl_next(iter)) {
		poly_t *poly = kl_val(iter);
		if(test(poly) > 0) {
			poly_t *clone = clone_poly(poly);
			check_mem(clone);
			*kl_pushp(poly, result) = clone;
		}
	}

	return result;
error:
	if((result != NULL) && (result != dst)) kl_destroy(poly, result);
	return NULL;
}

klist_t(poly) *map_polys_with_index(mesh_index_t *idx, klist_t(poly) *dst, klist_t(poly) *src, poly_with_index_t *mapper) {
	klist_t(poly) *to = NULL;
	if(dst == NULL) to = kl_init(poly);
	else to = dst;
	check_mem(to);

	kliter_t(poly) *iter = kl_begin(src);
	for(; iter != kl_end(src); iter = kl_next(iter)) {
		mapper(to, idx, kl_val(iter));
	}

	return to;
error:
	if((to != NULL) && (to != dst)) kl_destroy(poly, to);
	return NULL;
}
