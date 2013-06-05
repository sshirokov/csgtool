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
		poly_t *f = NULL;
		poly_t *b = NULL;
		check(poly_split(divider, poly, &f, &b) == 0,
			  "Failed to split polygon(%p) with divider(%p)", poly, divider);
		*kl_pushp(poly, front) = f;
		*kl_pushp(poly, back)  = b;
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
		int vertex_count = poly_vertex_count(poly);
		if(!make_triangles || vertex_count == 3) {
			poly_t *copy = clone_poly(poly);
			check_mem(copy);
			*kl_pushp(poly, dst) = copy;
		}
		else if(vertex_count > 3){
			// Start with the third vertex and build triangles
			// in in the form (v0, v_prev, v_cur)
			float3 *v_cur, *v_prev;
			for(int i = 2; i < vertex_count; i++) {
				v_cur = &poly->vertices[i];
				v_prev = &poly->vertices[i - 1];
				poly_t *tri = poly_make_triangle(poly->vertices[0], *v_prev, *v_cur);
				check_mem(tri);
				*kl_pushp(poly, dst) = tri;
			}
		}
		else {
			sentinel("polygon(%p) has less than three(%d) vertices.", poly, poly_vertex_count(poly));
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

klist_t(poly) *bsp_clip_polygons(bsp_node_t *node, klist_t(poly) *polygons) {
	klist_t(poly) *result = kl_init(poly);
	kliter_t(poly) *iter = NULL;
	poly_t *p = NULL;
	int rc = -1;

	if(node->divider != NULL) {
		klist_t(poly) *result_front = NULL;
		klist_t(poly) *result_back = NULL;

		// Sort this node's polygons into the front or back
		klist_t(poly) *node_front = kl_init(poly);
		klist_t(poly) *node_back = kl_init(poly);
		for(iter = kl_begin(polygons); iter != kl_end(polygons); iter = kl_next(iter)) {
			rc = bsp_subdivide(node->divider, kl_val(iter), node_front, node_back, node_front, node_back);
			check(rc != -1, "Failed to subdivide poly %p", kl_val(iter));
		}

		// Recur to the front tree, or copy my current front nodes to result.
		if(node->front) {
			result_front = bsp_clip_polygons(node->front, node_front);
			check(result_front != NULL, "Failed to clip front tree");
		}
		else {
			result_front = kl_init(poly);
			for(iter = kl_begin(node_front); iter != kl_end(node_front); iter = kl_next(iter)) {
				p = clone_poly(kl_val(iter));
				check_mem(p);
				*kl_pushp(poly, result_front) = p;
			}
		}

		// Repeat for the back tree
		if(node->back) {
			result_back = bsp_clip_polygons(node->back, node_back);
			check(result_back != NULL, "Failed to clip back tree");
		}
		else {
			result_back = kl_init(poly);
			for(iter = kl_begin(node_back); iter != kl_end(node_back); iter = kl_next(iter)) {
				p = clone_poly(kl_val(iter));
				check_mem(p);
				*kl_pushp(poly, result_back) = p;
			}
		}

		// Copy the entire front list into the result
		for(iter = kl_begin(result_front); iter != kl_end(result_front); iter = kl_next(iter)) {
			p = clone_poly(kl_val(iter));
			check_mem(p);
			*kl_pushp(poly, result) = p;
		}
		// Concat the back list if we have a back tree
		if(node->back != NULL) {
			for(iter = kl_begin(result_back); iter != kl_end(result_back); iter = kl_next(iter)) {
				p = clone_poly(kl_val(iter));
				check_mem(p);
				*kl_pushp(poly, result) = p;
			}
		}

		// Clean up the temporary lists
		kl_destroy(poly, node_front);
		kl_destroy(poly, node_back);
		// Clean up the result halves, now that they're copied into `result`
		kl_destroy(poly, result_front);
		kl_destroy(poly, result_back);
	}
	else {
		// If we don't have a divider we just copy out the polygons
		for(iter = kl_begin(polygons); iter != kl_end(polygons); iter = kl_next(iter)) {
			check_mem(p = clone_poly(kl_val(iter)));
			*kl_pushp(poly, result) = p;
		}
	}

	return result;
error:
	if(result) kl_destroy(poly, result);
	return NULL;
}

bsp_node_t *bsp_clip(bsp_node_t *us, bsp_node_t *them) {
	klist_t(poly) *new_polys = bsp_clip_polygons(them, us->polygons);
	check(new_polys != NULL, "Failed to generate new poly list in bsp_clip(%p, %p)", us, them);
	kl_destroy(poly, us->polygons);
	us->polygons = new_polys;

	if(us->front)
		check(bsp_clip(us->front, them) != NULL, "Failed to clip the front tree %p of %p", us->front, us);
	if(us->back)
		check(bsp_clip(us->back, them) != NULL, "Failed to clip the back tree %p of %p", us->back, us);

	return us;
error:
	if(new_polys) kl_destroy(poly, new_polys);
	return NULL;
}
