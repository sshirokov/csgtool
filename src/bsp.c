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
	poly_t *copy = NULL;
	switch(poly_classify_poly(divider, poly)) {
	case FRONT:
		check_mem(copy = clone_poly(poly));
		*kl_pushp(poly, front) = copy;
		break;
	case BACK:
		check_mem(copy = clone_poly(poly));
		*kl_pushp(poly, back) = copy;
		break;
	case COPLANAR:
		check_mem(copy = clone_poly(poly));
		if(f3_dot(divider->normal, poly->normal) > 0)
			*kl_pushp(poly, coplanar_front) = copy;
		else
			*kl_pushp(poly, coplanar_back) = copy;
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
	poly_t *poly = NULL;

	if(node == NULL) {
		// Allocate a node if we weren't given one. It's the nice
		// thing to do for people.
		node = alloc_bsp_node();
		check_mem(node);
	}

	if(node->divider == NULL) {
		// Add the divider to the list of coplanar polygons
		// and advance the iterator to the next polygon
		// if we have not yet picked the divider for this node.
		// This avoids having to rely on an explicit
		// test of this node against itself in the loop below.
		check_mem(poly = clone_poly(kl_val(iter)));
		*kl_pushp(poly, node->polygons) = poly;
		iter = kl_next(iter);

		node->divider = clone_poly(kl_val(kl_begin(polygons)));
		check_mem(node->divider);
	}


	int rc = 0;
	for(; iter != kl_end(polygons); iter = kl_next(iter)) {
		poly = kl_val(iter);
		rc = bsp_subdivide(node->divider, poly, node->polygons, node->polygons, front, back);
		check(rc == 0, "Failed to subdivide: %p => %p", node->divider, poly);
	}

	// Leaving this trace here just because I've toggled it a lot
	// log_info("bsp_build(%zd): %zd COPLANAR, %zd front, %zd back", polygons->size, node->polygons->size, front->size, back->size);

	if((front->size > 0)) {
		// log_info("\tBuilding front of %p->%p", node, node->front);
		if(node->front == NULL) node->front = alloc_bsp_node();
		check_mem(node->front);
		check(bsp_build(node->front, front) != NULL,
			  "Failed to build front tree of bsp_node(%p)", node);
	}
	if((back->size > 0)) {
		// log_info("\tBuilding back of %p->%p", node, node->back);
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

int bsp_copy_node_polygons(bsp_node_t *node, int make_triangles, klist_t(poly) *dst) {
	int copied = 0;
	kliter_t(poly) *iter = kl_begin(node->polygons);
	for(;iter != kl_end(node->polygons); iter = kl_next(iter)) {
		poly_t *poly = kl_val(iter);
		if(!make_triangles || poly->vertices->size == 3) {
			poly_t *copy = clone_poly(poly);
			check_mem(copy);
			*kl_pushp(poly, dst) = copy;
		}
		else if(poly->vertices->size > 3){
			// Start with the third vertex and build triangles
			// in in the form (v0, v_prev, v_cur)
			kliter_t(float3) *v_cur = kl_next(kl_next(kl_begin(poly->vertices)));
			kliter_t(float3) *v_prev = kl_next(kl_begin(poly->vertices));
			for(;v_cur != kl_end(poly->vertices); v_cur = kl_next(v_cur), v_prev = kl_next(v_prev)) {
				poly_t *tri = poly_make_triangle(*kl_val(kl_begin(poly->vertices)),
												 *kl_val(v_prev),
												 *kl_val(v_cur));
				check_mem(tri);
				*kl_pushp(poly, dst) = tri;
			}
		}
		else {
			sentinel("polygon(%p) has less than three(%zd) vertices.", poly, poly->vertices->size);
		}
		copied++;
	}
	return copied;
error:
	return -1;
}

klist_t(poly) *bsp_to_polygons(bsp_node_t *tree, int make_triangles, klist_t(poly) *dst) {
	klist_t(poly) *polygons = dst ? dst : kl_init(poly);

	if(tree->back != NULL) {
		bsp_to_polygons(tree->back, make_triangles, polygons);
	}

	int rc = bsp_copy_node_polygons(tree, make_triangles, polygons);
	check(rc == tree->polygons->size, "bsp_copy_node_polygons() did not copy all polygons");

	if(tree->front != NULL) {
		bsp_to_polygons(tree->front, make_triangles, polygons);
	}

	return polygons;
error:
	// Only clean up the polygons list if we initialized it on error
	if(dst == NULL) kl_destroy(poly, polygons);
	return NULL;
}

bsp_node_t *bsp_invert(bsp_node_t *tree) {
	// Invert every polygon in this node
	poly_t *poly = NULL;
	kliter_t(poly) *iter = kl_begin(tree->polygons);
	for(; iter != kl_end(tree->polygons); iter = kl_next(iter)) {
		poly = poly_invert(kl_val(iter));
		check(poly != NULL, "Failed to invert polygon %p", kl_val(iter));
	}

	// Invert the divider
	if(tree->divider) {
		poly = poly_invert(tree->divider);
		check(poly != NULL, "Failed to inverts bsp_node_t(%p)->divider(%p)", tree, tree->divider);
	}

	// Invert the front and back trees
	bsp_node_t *node;
	if(tree->front) {
		node = bsp_invert(tree->front);
		check(node != NULL, "Failed to invert back tree of bsp_node_t(%p)", tree->front);
	}
	if(tree->back) {
		node = bsp_invert(tree->back);
		check(node != NULL, "Failed to invert back tree of bsp_node_t(%p)", tree->back);
	}

	// Swap front and back trees
	node = tree->front;
	tree->front = tree->back;
	tree->back = node;

	return tree;
error:
	return NULL;
}
