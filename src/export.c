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
