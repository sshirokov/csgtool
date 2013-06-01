#include "bsp.h"
#include "dbg.h"

bsp_node_t *alloc_bsp_node(void) {
	bsp_node_t *node = NULL;
	check_mem(node = calloc(1, sizeof(bsp_node_t)));
	node->polygons = kl_init(poly);
	return node;
error:
	return NULL;
}

void free_bsp_node(bsp_node_t *node) {
	kl_destroy(poly, node->polygons);
	free_bsp_node(node->front);
	free_bsp_node(node->back);
}

int bsp_subdivide(poly_t *divider, poly_t *poly,
				   klist_t(poly) *coplanar_front, klist_t(poly) *coplanar_back,
				   klist_t(poly) *front, klist_t(poly) *back) {
	switch(poly_classify_poly(divider, poly)) {
	case FRONT:
		*kl_pushp(poly, front) = poly;
		break;
	case BACK:
		*kl_pushp(poly, back) = poly;
		break;
	case COPLANAR:
		if(f3_dot(divider->normal, poly->normal) > 0)
			*kl_pushp(poly, coplanar_front) = poly;
		else
			*kl_pushp(poly, coplanar_back) = poly;
		break;
	case SPANNING: {
		poly_t *front_back = NULL;
		check_mem(front_back = poly_split(divider, poly));
		*kl_pushp(poly, front) = &front_back[0];
		*kl_pushp(poly, back)  = &front_back[1];
		break;
	}
	}

	return 0;
error:
	return -1;
}

bsp_node_t *bsp_build(bsp_node_t *node, klist_t(poly) *polygons) {
	kliter_t(poly) *iter = kl_begin(polygons);
	klist_t(poly) *front = kl_init(poly);
	klist_t(poly) *back  = kl_init(poly);

	if(node->divider == NULL) {
		// Add the divider to the list of coplanar polygons
		// and advance the iterator to the next polygon
		// if we have not yet picked the divider for this node.
		// This avoids having to rely on an explicit
		// test of this node against itself in the loop below.
	   *kl_pushp(poly, node->polygons) = kl_val(iter);
	   iter = kl_next(iter);

		node->divider = clone_poly(kl_val(kl_begin(polygons)));
		check_mem(node->divider);
	}


	poly_t *poly = NULL;
	int rc = 0;
	for(; iter != kl_end(polygons); iter = kl_next(iter)) {
		poly = kl_val(iter);
		rc = bsp_subdivide(node->divider, poly, node->polygons, node->polygons, front, back);
		check(rc == 0, "Failed to subdivide: %p => %p", node->divider, poly);
	}

	log_info("bsp_build(%zd): %zd COPLANAR, %zd front, %zd back", polygons->size, node->polygons->size, front->size, back->size);

	if((front->size > 0)) {
		log_info("\tBuilding front of %p->%p", node, node->front);
		if(node->front == NULL) node->front = alloc_bsp_node();
		check_mem(node->front);
		check(bsp_build(node->front, front) != NULL,
			  "Failed to build front tree of bsp_node(%p)", node);
	}
	if((back->size > 0)) {
		log_info("\tBuilding back of %p->%p", node, node->back);
		if(node->back == NULL) node->back = alloc_bsp_node();
		check_mem(node->back);
		check(bsp_build(node->back, back) != NULL,
			  "Failed to build back tree of bsp_node(%p)", node);
	}

	return node;
error:
	kl_destroy(poly, front);
	kl_destroy(poly, back);
	return NULL;
}

int bsp_copy_node_polygons(bsp_node_t *node, klist_t(poly) *dst) {
	int copied = 0;
	kliter_t(poly) *iter = kl_begin(node->polygons);
	for(;iter != kl_end(node->polygons); iter = kl_next(iter)) {
		poly_t *copy = clone_poly(kl_val(iter));
		check_mem(copy);
		*kl_pushp(poly, dst) = copy;
		copied++;
	}
	return copied;
error:
	return -1;
}

klist_t(poly) *bsp_to_polygons(bsp_node_t *tree, klist_t(poly) *dst) {
	klist_t(poly) *polygons = dst ? dst : kl_init(poly);

	if(tree->back != NULL)
		bsp_to_polygons(tree->back, polygons);

	bsp_copy_node_polygons(tree, polygons);

	if(tree->front != NULL)
		bsp_to_polygons(tree->front, polygons);


	return polygons;
}
