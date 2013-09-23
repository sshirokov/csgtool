#include <assert.h>

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

bsp_node_t *clone_bsp_tree(bsp_node_t *tree) {
	bsp_node_t *copy = alloc_bsp_node();
	check_mem(copy);

	kliter_t(poly) *iter = kl_begin(tree->polygons);
	for(; iter != kl_end(tree->polygons); iter = kl_next(iter)) {
		poly_t *poly_copy = clone_poly(kl_val(iter));
		check_mem(poly_copy);
		*kl_pushp(poly, copy->polygons) = poly_copy;
	}

	free_poly(copy->divider, 1);
	if(tree->divider) {
		copy->divider = clone_poly(tree->divider);
		check_mem(copy->divider);
	}
	else {
		copy->divider = NULL;
	}

	if(tree->front != NULL) {
		copy->front = clone_bsp_tree(tree->front);
		check_mem(copy->front);
	}
	if(tree->back != NULL) {
		copy->back = clone_bsp_tree(tree->back);
		check_mem(copy->back);
	}

	return copy;
error:
	if(copy != NULL) free_bsp_tree(copy);
	return NULL;
}

void free_bsp_node(bsp_node_t *node) {
	if(node == NULL) return;
	kl_destroy(poly, node->polygons);
	free_poly(node->divider, 1);
	free(node);
}

void free_bsp_tree(bsp_node_t *tree) {
	if(tree == NULL) return;
	if(tree->front != NULL) free_bsp_tree(tree->front);
	if(tree->back != NULL) free_bsp_tree(tree->back);
	free_bsp_node(tree);
}

// Put the polygon in the the appropriate list
// and increment the counter assosiated with it.
// A polygon can end up in muliple lists, but not
// all of them.
int bsp_subdivide(poly_t *divider, poly_t *poly,
				  poly_t **coplanar_front, int *n_cp_front,
				  poly_t **coplanar_back,  int *n_cp_back,
				  poly_t **front,          int *n_front,
				  poly_t **back,           int *n_back,
				  poly_t **unused,         int *n_unused,
				  poly_t **created,        int *n_created) {
	switch(poly_classify_poly(divider, poly)) {
	case FRONT:
		front[*n_front] = poly;
		*n_front += 1;
		break;
	case BACK:
		back[*n_back] = poly;
		*n_back += 1;
		break;
	case COPLANAR:
		if(f3_dot(divider->normal, poly->normal) > 0) {
			coplanar_front[*n_cp_front] = poly;
			*n_cp_front += 1;
		}
		else {
			coplanar_back[*n_cp_back] = poly;
			*n_cp_back += 1;
		}
		break;
	case SPANNING: {
		poly_t *f = NULL;
		poly_t *b = NULL;
		check(poly_split(divider, poly, &f, &b) == 0,
			  "Failed to split polygon(%p) with divider(%p)", poly, divider);
		front[*n_front] = f;
		*n_front += 1;

		back[*n_back] = b;
		*n_back += 1;

		// Do we care about telling the caller about polygons
		// who's pointers are not in any of the "real" lists?
		if(unused != NULL) {
			unused[*n_unused] = poly;
			*n_unused += 1;
		}

		// How about polygons that we just made?
		if(created != NULL) {
			created[*n_created] = f;
			*n_created += 1;
			created[*n_created] = b;
			*n_created += 1;
		}
		break;
	}
	}

	return 0;
error:
	return -1;
}

bsp_node_t *bsp_build(bsp_node_t *node, klist_t(poly) *polygons, int copy) {
	poly_t **polys = NULL;
	check_mem(polys = malloc(sizeof(poly_t*) * polygons->size));

	kliter_t(poly) *iter = NULL;
	int i = 0;
	for(iter = kl_begin(polygons); iter != kl_end(polygons); i++, iter = kl_next(iter)) {
		poly_t *poly = NULL;
		poly = copy ? clone_poly(kl_val(iter)) : kl_val(iter);
		check(poly != NULL, "Failed to make poly array. Item %d is NULL in list %p. (Copy: %s)",
			  i, polygons, copy ? "true" : "false");
		polys[i] = poly;
	}

	check((node = bsp_build_array(node, polys, polygons->size, copy)),
		  "Failed to build node from list(%p) of %zd polys", polygons, polygons->size);
	free(polys);

	return node;
error:
	if(polys) free(polys);
	return NULL;
}

bsp_node_t *bsp_build_array(bsp_node_t *node, poly_t **polygons, size_t n_polys, int free_unused) {
	int rc = 0;

	// Polygon lists and counters
	int n_coplanar = 0;
	int n_front = 0;
	int n_back = 0;
	poly_t **coplanar = NULL;
	poly_t **front_p  = NULL;
	poly_t **back_p   = NULL;

	// List and counter of unused polygons
	// These will get freed after the build
	// because they will not appear with identity
	// in coplanar, front_p, or back_p if free_unused
	// is true
	int n_unused = 0;
	poly_t **unused = NULL;

	// Iterators
	poly_t *poly = NULL;
	size_t poly_i = 0;

	if(node == NULL) {
		// Allocate a node if we weren't given one. It's the nice
		// thing to do for people.
		node = alloc_bsp_node();
		check_mem(node);
	}

	if(n_polys == 0) return node;

	if(node->divider == NULL) {
		// Add the divider to the list of coplanar polygons
		// and advance the iterator to the next polygon
		// if we have not yet picked the divider for this node.
		// This avoids having to rely on an explicit
		// test of this node against itself in the loop below.
		*kl_pushp(poly, node->polygons) = polygons[0];
		poly_i += 1;

		node->divider = clone_poly(polygons[0]);
		check_mem(node->divider);
	}


	check_mem(coplanar = malloc(sizeof(poly_t*) * n_polys));
	check_mem(front_p = malloc(sizeof(poly_t*) * n_polys));
	check_mem(back_p = malloc(sizeof(poly_t*) * n_polys));
	check_mem(unused = malloc(sizeof(poly_t*) * n_polys));
	for(; poly_i < n_polys; poly_i++) {
		poly = polygons[poly_i];
		rc = bsp_subdivide(node->divider, poly,
						   coplanar, &n_coplanar,
						   coplanar, &n_coplanar,
						   front_p, &n_front,
						   back_p, &n_back,
						   unused, &n_unused,
						   NULL, NULL);
		check(rc == 0, "Failed to subdivide: %p => %p", node->divider, poly);
	}

	// Destroy the unused polygons now, if we're asked,
	// otherwise we'll lose the references
	int i = 0;
	if(free_unused != 0) {
		for(i = 0; i < n_unused; i++) {
			free_poly(unused[i], 1);
		}
	}
	// Free now and mark NULL to make sure it's not double free'd on `error:`
	free(unused);
	unused = NULL;

	// Store the coplanar nodes in this node's polygon list
	// and free the container, letting the list destructor
	// clean up
	for(i = 0; i < n_coplanar; i++) {
		*kl_pushp(poly, node->polygons) = coplanar[i];
	}
	free(coplanar);
	coplanar = NULL;

	if((n_front > 0)) {
		if(node->front == NULL) node->front = alloc_bsp_node();
		check_mem(node->front);
		check(bsp_build_array(node->front, front_p, n_front, free_unused) != NULL,
			  "Failed to build front tree of bsp_node_array(%p)", node);
	}
	if((n_back > 0)) {
		if(node->back == NULL) node->back = alloc_bsp_node();
		check_mem(node->back);
		check(bsp_build_array(node->back, back_p, n_back, free_unused) != NULL,
			  "Failed to build back tree of bsp_node(%p)", node);
	}
	free(front_p);
	free(back_p);
	front_p = back_p = NULL;

	return node;
error:
	if(coplanar) free(coplanar);
	if(back_p) free(back_p);
	if(front_p) free(front_p);
	if(unused) free(unused);
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

klist_t(poly) *bsp_clip_polygon_array(bsp_node_t *node, poly_t **polygons, size_t n_polys, klist_t(poly) *dst) {
	klist_t(poly) *result = dst != NULL ? dst : kl_init(poly);
	size_t i;
	poly_t *p = NULL;
	int rc = -1;

	poly_t *static_poly_buffer[STATIC_POLY_BUFFER_SIZE];
	poly_t **poly_buffer = static_poly_buffer;
	poly_t **front_array = NULL;
	poly_t **back_array = NULL;
	poly_t **created_array = NULL;
	int n_front = 0;
	int n_back = 0;
	int n_created = 0;

	// Let's end this quick if there's nothing to do.
	if(n_polys == 0) return result;

	if(node->divider != NULL) {
		if((n_polys * 3) > STATIC_POLY_BUFFER_SIZE) {
			check_mem(poly_buffer = malloc((sizeof(poly_t*) * n_polys) * 3));
		}
		front_array = poly_buffer;
		back_array = poly_buffer + n_polys;
		created_array = poly_buffer + (n_polys * 2);
		// Sort this node's polygons into the front or back
		for(i = 0; i < n_polys; i++) {
			p = polygons[i];
			rc = bsp_subdivide(node->divider, p,
							   front_array, &n_front, back_array, &n_back,
							   front_array, &n_front, back_array, &n_back,
							   NULL, NULL, created_array, &n_created);
			check(rc != -1, "Failed to subdivide poly %p", p);
		}

		int i;
		poly_t *copy = NULL;
		// Recur to the front tree, or copy my current front nodes to result.
		if(node->front) {
			result = bsp_clip_polygon_array(node->front, front_array, n_front, result);
			check(result != NULL, "Failed to clip front tree");
		}
		else {
			for(i = 0; i < n_front; i++) {
				copy = clone_poly(front_array[i]);
				check_mem(copy);
				*kl_pushp(poly, result) = copy;
			}
		}

		// Repeat for the back tree
		if(node->back) {
			result = bsp_clip_polygon_array(node->back, back_array, n_back, result);
			check(result != NULL, "Failed to clip back tree");
		}

		// Free all the polygons in 'created_array` since they would have
		// been cloned if they were important, and the input set is not our
		// responsibility
		for(int j = 0; j < n_created; j++) {
			free_poly(created_array[j], 1);
		}

		// Clean up the result halves, now that they're copied into `result`
		if(poly_buffer != static_poly_buffer) free(poly_buffer);
	}
	else {
		// If we don't have a divider we just copy out the polygons
		for(i = 0; i < n_polys; i++) {
			check_mem(p = clone_poly(polygons[i]));
			*kl_pushp(poly, result) = p;
		}
	}

	return result;
error:
	if(poly_buffer != static_poly_buffer) free(poly_buffer);
	if(result) kl_destroy(poly, result);
	return NULL;
}

klist_t(poly) *bsp_clip_polygons(bsp_node_t *node, klist_t(poly) *polygons, klist_t(poly) *dst) {
	klist_t(poly) *result = dst != NULL ? dst : kl_init(poly);
	kliter_t(poly) *iter = NULL;
	poly_t *p = NULL;
	int rc = -1;

	poly_t *static_poly_buffer[STATIC_POLY_BUFFER_SIZE];
	poly_t **poly_buffer = static_poly_buffer;
	poly_t **front_array = NULL;
	poly_t **back_array = NULL;
	poly_t **created_array = NULL;
	int n_front = 0;
	int n_back = 0;
	int n_created = 0;

	// Let's end this quick if there's nothing to do.
	if(polygons->size == 0) return result;

	if(node->divider != NULL) {
		if((polygons->size * 3) > STATIC_POLY_BUFFER_SIZE) {
			check_mem(poly_buffer = malloc(sizeof(poly_t*) * polygons->size * 3));
		}
		front_array = poly_buffer;
		back_array = poly_buffer + polygons->size;
		created_array = poly_buffer + (polygons->size * 2);
		// Sort this node's polygons into the front or back
		for(iter = kl_begin(polygons); iter != kl_end(polygons); iter = kl_next(iter)) {
			rc = bsp_subdivide(node->divider, kl_val(iter),
							   front_array, &n_front, back_array, &n_back,
							   front_array, &n_front, back_array, &n_back,
							   NULL, NULL, created_array, &n_created);
			check(rc != -1, "Failed to subdivide poly %p", kl_val(iter));
		}

		int i;
		poly_t *copy = NULL;
		// Recur to the front tree, or copy my current front nodes to result.
		if(node->front) {
			result = bsp_clip_polygon_array(node->front, front_array, n_front, result);
			check(result != NULL, "Failed to clip front tree");
		}
		else {
			for(i = 0; i < n_front; i++) {
				copy = clone_poly(front_array[i]);
				check_mem(copy);
				*kl_pushp(poly, result) = copy;
			}
		}

		// Repeat for the back tree
		if(node->back) {
			result = bsp_clip_polygon_array(node->back, back_array, n_back, result);
			check(result != NULL, "Failed to clip back tree");
		}

		// Free all the polygons in 'created_array` since they would have
		// been cloned if they were important, and the input set is not our
		// responsibility
		for(int j = 0; j < n_created; j++) {
			free_poly(created_array[j], 1);
		}


		if(poly_buffer != static_poly_buffer) free(poly_buffer);
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
	if(poly_buffer != static_poly_buffer) free(poly_buffer);
	if(result) kl_destroy(poly, result);
	return NULL;
}

bsp_node_t *bsp_clip(bsp_node_t *us, bsp_node_t *them) {
	klist_t(poly) *new = bsp_clip_polygons(them, us->polygons, NULL);
	kl_destroy(poly, us->polygons);
	us->polygons = new;

	if(us->front != NULL) bsp_clip(us->front, them);
	if(us->back != NULL) bsp_clip(us->back, them);

	return us;
}

bsp_node_t *bsp_subtract(bsp_node_t *tree_a, bsp_node_t *tree_b) {
	bsp_node_t *a = NULL;
	bsp_node_t *b = NULL;
	bsp_node_t *result = NULL;
	klist_t(poly) *b_polys = NULL;

	check_mem(a = clone_bsp_tree(tree_a));
	check_mem(b = clone_bsp_tree(tree_b));

	check(bsp_invert(a)  != NULL, "Failed to invert A");
	check(bsp_clip(a, b) != NULL, "Failed to clip(A, B)");
	check(bsp_clip(b, a) != NULL, "Failed clip(B, A)");
	check(bsp_invert(b)  != NULL, "Failed to invert B")
	check(bsp_clip(b, a) != NULL, "Failed to clio(B, A)");
	check(bsp_invert(b)  != NULL, "Failed to invert B")
	b_polys = bsp_to_polygons(b, 0, NULL);
	check(b_polys != NULL, "Failed to get polygons from B");
	check(bsp_build(a, b_polys, 1) == a, "Failed to add nodes from B into tree A");
	check(bsp_invert(a)  != NULL, "Failed to invert A")

	// TODO: Build a more balanced trees from the polys of
	//       a instead of cloning a tree with potential gaps.
	result = clone_bsp_tree(a);
	check_mem(result);

	if(b_polys != NULL) kl_destroy(poly, b_polys);
	if(a != NULL) free_bsp_tree(a);
	if(b != NULL) free_bsp_tree(b);
	return result;
error:
	if(b_polys != NULL) kl_destroy(poly, b_polys);
	if(a != NULL) free_bsp_tree(a);
	if(b != NULL) free_bsp_tree(b);
	if(result != NULL) free_bsp_tree(result);
	return NULL;
}

bsp_node_t *bsp_union(bsp_node_t *tree_a, bsp_node_t *tree_b) {
	bsp_node_t *a = NULL;
	bsp_node_t *b = NULL;
	bsp_node_t *result = NULL;
	klist_t(poly) *b_polys = NULL;

	check_mem(a = clone_bsp_tree(tree_a));
	check_mem(b = clone_bsp_tree(tree_b));

	check(bsp_clip(a, b) != NULL, "Failed to clip(A, B)");
	check(bsp_clip(b, a) != NULL, "Failed clip(B, A)");
	check(bsp_invert(b)  != NULL, "Failed to invert B")
	check(bsp_clip(b, a) != NULL, "Failed to clio(B, A)");
	check(bsp_invert(b)  != NULL, "Failed to invert B")
	b_polys = bsp_to_polygons(b, 0, NULL);
	check(b_polys != NULL, "Failed to get polygons from B");
	check(bsp_build(a, b_polys, 1) == a, "Failed to add nodes from B into tree A");

	// TODO: Build a more balanced trees from the polys of
	//       a instead of cloning a tree with potential gaps.
	result = clone_bsp_tree(a);
	check_mem(result);

	if(b_polys != NULL) kl_destroy(poly, b_polys);
	if(a != NULL) free_bsp_tree(a);
	if(b != NULL) free_bsp_tree(b);
	return result;
error:
	if(b_polys != NULL) kl_destroy(poly, b_polys);
	if(a != NULL) free_bsp_tree(a);
	if(b != NULL) free_bsp_tree(b);
	if(result != NULL) free_bsp_tree(result);
	return NULL;
}

bsp_node_t *bsp_intersect(bsp_node_t *tree_a, bsp_node_t *tree_b) {
	bsp_node_t *a = NULL;
	bsp_node_t *b = NULL;
	bsp_node_t *result = NULL;
	klist_t(poly) *b_polys = NULL;

	check_mem(a = clone_bsp_tree(tree_a));
	check_mem(b = clone_bsp_tree(tree_b));

	check(bsp_invert(a)  != NULL, "Failed to invert A");
	check(bsp_clip(b, a) != NULL, "Failed clip(B, A)");
	check(bsp_invert(b)  != NULL, "Failed to invert B");
	check(bsp_clip(a, b) != NULL, "Failed to clip(A, B)");
	check(bsp_clip(b, a) != NULL, "Failed to clio(B, A)");

	b_polys = bsp_to_polygons(b, 0, NULL);
	check(b_polys != NULL, "Failed to get polygons from B");
	check(bsp_build(a, b_polys, 1) == a, "Failed to add nodes from B into tree A");
	check(bsp_invert(a) == a, "Failed to invert tree A");

	// TODO: Build a more balanced trees from the polys of
	//       a instead of cloning a tree with potential gaps.
	result = clone_bsp_tree(a);
	check_mem(result);

	if(b_polys != NULL) kl_destroy(poly, b_polys);
	if(a != NULL) free_bsp_tree(a);
	if(b != NULL) free_bsp_tree(b);
	return result;
error:
	if(b_polys != NULL) kl_destroy(poly, b_polys);
	if(a != NULL) free_bsp_tree(a);
	if(b != NULL) free_bsp_tree(b);
	if(result != NULL) free_bsp_tree(result);
	return NULL;
}
