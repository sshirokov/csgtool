#include "export.h"

stl_object *stl_from_polys(klist_t(poly) *polygons) {
	stl_object *stl = stl_alloc(NULL, polygons->size);
	check_mem(stl);

	kliter_t(poly) *iter = kl_begin(polygons);
	stl_facet *facet = stl->facets;
	poly_t *poly = NULL;
	for(; iter != kl_end(polygons); iter = kl_next(iter), facet++) {
		poly = kl_val(iter);
		check(poly_vertex_count(poly) == 3, "Polygon is not a triangle.");
		memcpy(facet->normal, poly->normal, sizeof(float3));
		memcpy(facet->vertices, poly->vertices, sizeof(facet->vertices));
	}

	return stl;
error:
	if(stl) stl_free(stl);
	return NULL;
}

stl_object *bsp_to_stl(bsp_node_t *tree) {
	stl_object *stl = NULL;
	klist_t(poly) *polys = NULL;

	polys = bsp_to_polygons(tree, 1, NULL);
	check(polys != NULL, "Failed to generate polygons from bsp_node_t(%p)", tree);

	stl = stl_from_polys(polys);
	check(stl != NULL, "Failed to build stl from %zd polygons", polys->size);
	strcpy(stl->header, "[csgtool BSP output]");

	kl_destroy(poly, polys);
	return stl;
error:
	if(polys) kl_destroy(poly, polys);
	if(stl) stl_free(stl);
	return NULL;
}

bsp_node_t *stl_to_bsp(stl_object *stl) {
	bsp_node_t *tree = NULL;
	klist_t(poly) *polys = kl_init(poly);
	poly_t *poly = NULL;

	for(int i = 0; i < stl->facet_count; i++) {
		poly = poly_make_triangle(stl->facets[i].vertices[0],
								  stl->facets[i].vertices[1],
								  stl->facets[i].vertices[2]);
		check_mem(polys);
		*kl_pushp(poly, polys) = poly;
	}
	check(polys->size == stl->facet_count, "Wrong number of faces generated.");

	tree = bsp_build(NULL, polys, 1);
	check_mem(tree);

	kl_destroy(poly, polys);
	return tree;
error:
	if(polys != NULL) kl_destroy(poly, polys);
	if(tree != NULL) {
		free_bsp_tree(tree);
	}
	return NULL;
}

bsp_node_t *mesh_to_bsp(mesh_t *mesh) {
	bsp_node_t *tree = NULL;
	klist_t(poly) *polys = NULL;

	check((polys = mesh->to_polygons(mesh)) != NULL,
		  "Failed to convert mesh %p to polgon list", mesh);

	check((tree = bsp_build(NULL, polys, 1)) != NULL,
		  "Failed to build BSP tree from %zd polys", polys->size);

	kl_destroy(poly, polys);
	return tree;
error:
	if(tree != NULL) free_bsp_tree(tree);
	if(polys != NULL) kl_destroy(poly, polys);
	return NULL;
}

klist_t(poly)* polys_to_tris(klist_t(poly) *dst, klist_t(poly) *src) {
	klist_t(poly) *result = dst;
	if(result == NULL) result = kl_init(poly);

	kliter_t(poly) *iter = NULL;
	for(iter = kl_begin(src); iter != kl_end(src); iter = kl_next(iter)) {
		poly_t *poly = kl_val(iter);
		int vertex_count = poly_vertex_count(poly);

		// Copy triangles, split higher-vertex polygons into triangle fans.
		if(vertex_count == 3) {
			poly_t *copy = clone_poly(poly);
			check_mem(copy);
			*kl_pushp(poly, result) = copy;
		}
		else if(vertex_count > 3) {
			float3 *v_cur, *v_prev;
			for(int i = 2; i < vertex_count; i++) {
				v_cur = &poly->vertices[i];
				v_prev = &poly->vertices[i - 1];
				poly_t *tri = poly_make_triangle(poly->vertices[0], *v_prev, *v_cur);
				check_mem(tri);
				*kl_pushp(poly, result) = tri;
			}
		} else {
			sentinel("polygon(%p) has less than three(%d) vertices.", poly, poly_vertex_count(poly));
		}
	}

	return result;
error:
	if(result != dst) if(result != NULL) kl_destroy(poly, result);
	return NULL;
}
