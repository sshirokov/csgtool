#include "export.h"

stl_object *stl_from_polys(klist_t(poly) *polygons) {
	stl_object *stl = stl_alloc(NULL, polygons->size);
	check_mem(stl);

	kliter_t(poly) *iter = kl_begin(polygons);
	stl_facet *facet = stl->facets;
	poly_t *poly = NULL;
	for(; iter != kl_end(polygons); iter = kl_next(iter), facet++) {
		poly = kl_val(iter);
		check(poly->vertices->size == 3, "Polygon is not a triangle.");
		memcpy(facet->normal, poly->normal, sizeof(float3));

		kliter_t(float3) *viter = kl_begin(poly->vertices);
		for(int i = 0; i < 3; i++, viter = kl_next(viter)) {
			memcpy(&facet->vertices[i], kl_val(viter), sizeof(float3));
		}
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
	check(polys->size > 0, "No polygons returned from tree(%p)", tree);

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

	tree = bsp_build(NULL, polys);
	check_mem(tree);

	kl_destroy(poly, polys);
	return tree;
error:
	if(polys != NULL) kl_destroy(poly, polys);
	if(tree != NULL) {
		log_warn("TODO: free_bsp()");
	}
	return NULL;
}
