#include "clar.h"

#include "stl.h"
#include "vector.h"
#include "poly.h"

poly_t *poly = NULL;
poly_t *poly_clone = NULL;

char jaws[] = CLAR_FIXTURE_PATH "jaws.stl";
stl_object *jaws_stl = NULL;

poly_t *make_triangle(float3 a, float3 b, float3 c) {
	poly_t *p = alloc_poly();
	if(p == NULL) return NULL;
	*kl_pushp(float3, p->vertices) = clone_f3(a);
	*kl_pushp(float3, p->vertices) = clone_f3(b);
	*kl_pushp(float3, p->vertices) = clone_f3(c);
	poly_update(p);
	return p;
}


void test_classify__initialize(void) {
	float3 vs[] = {{-1.0, -1.0, 0.0},
				   {1.0, -1.0, 0.0},
				   {0.0, 1.0, 0.0}};
	poly = make_triangle(vs[0], vs[1], vs[2]);
	cl_assert_(poly != NULL, "Out of memory");
	float total = poly->normal[0] + poly->normal[1] + poly->normal[2];
	cl_assert_(total > 0.0, "Normal should not be null");

	poly_clone = clone_poly(poly);
	cl_assert_(poly_clone != NULL, "Failed to clone polygon.");

	// Load up the jaws model
	jaws_stl = stl_read_file(jaws, 0);
	cl_assert_(jaws_stl != NULL, "Failed to parse jaws.");
}

void test_classify__cleanup(void) {
	if(poly) free_poly(poly);
	if(poly_clone) free_poly(poly_clone);
	if(jaws_stl) stl_free(jaws_stl);
}

void test_classify__polygon_vertices_coplanar(void) {
	float3 *v = kl_val(kl_begin(poly->vertices));

	int side = poly_classify_vertex(poly, *v);
	cl_assert_equal_i(side, COPLANAR);
}

void test_classify__polygon_self_dupe_coplanar(void) {
	int side = poly_classify_poly(poly, poly_clone);
	cl_assert_equal_i(side, COPLANAR);
}

void test_classify__polygon_tilted_dupe_coplanar(void) {
	int rc = 0;

	poly_t *another = clone_poly(poly);
	(*kl_val(kl_begin(another->vertices)))[2] += 0.6;
	cl_must_pass(poly_update(another));
	poly_t *another_clone = clone_poly(another);
	cl_must_pass(poly_update(another_clone));

	rc = poly_classify_poly(another, another_clone);
	cl_assert_equal_i(rc, COPLANAR);

	if(another) free_poly(another);
	if(another_clone) free_poly(another_clone);
}
