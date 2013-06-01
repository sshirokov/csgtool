#include "clar.h"

#include "stl.h"
#include "vector.h"
#include "poly.h"

poly_t *poly = NULL;
poly_t *poly_perp = NULL;
poly_t *poly_clone = NULL;

char jaws[] = CLAR_FIXTURE_PATH "jaws.stl";
stl_object *jaws_stl = NULL;

void test_classify__initialize(void) {
	float3 vs[] = {{-1.0, -1.0, 0.0},
				   {1.0, -1.0, 0.0},
				   {0.0, 1.0, 0.0}};
	poly = poly_make_triangle(vs[0], vs[1], vs[2]);
	cl_assert_(poly != NULL, "Out of memory");

	float3 splitvs[] = {{0.0, 1.0, 0.0},
						{0.0, -1.0, 0.0},
						{0.0, 0.0, 1.0}};
	poly_perp = poly_make_triangle(splitvs[0], splitvs[1], splitvs[2]);
	cl_assert_(poly_perp, "Out of memory");

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

	// Are we sane?
	rc = poly_classify_poly(poly, another);
	cl_assert_(rc != COPLANAR, "Polys should not be coplanar when one vertex is lifted off Z");

	rc = poly_classify_poly(another, another_clone);
	cl_assert_equal_i(rc, COPLANAR);

	if(another) free_poly(another);
	if(another_clone) free_poly(another_clone);
}

void test_classify__polygon_spanning(void) {
	int rc = 0;
	rc = poly_classify_poly(poly_perp, poly);
	cl_assert_equal_i(rc, SPANNING);
}

void test_classify__polygon_split(void) {
	poly_t *front_back = poly_split(poly_perp, poly);
	cl_assert(front_back != NULL);

	int rc = 0;
	rc = poly_classify_poly(poly_perp, &front_back[0]);
	cl_assert_(rc == FRONT, "Front poly of polygon split is not in the front.");
	rc = poly_classify_poly(poly_perp, &front_back[1]);
	cl_assert_(rc == BACK, "Back poly of polygon split is not in the back.");

	if(front_back != NULL) {
		if(&front_back[0]) free_poly(&front_back[0]);
		if(&front_back[1]) free_poly(&front_back[1]);
	}
}

void test_classify__jaws_polys_clone_coplanar(void) {
	cl_assert_(jaws_stl->facet_count > 0, "Mr.Jaws should have some faces.");
	for(size_t i = 0; i < jaws_stl->facet_count; i++) {
		int rc = 0;
		stl_facet *face = &jaws_stl->facets[i];
		poly_t *poly = poly_make_triangle(face->vertices[0], face->vertices[1], face->vertices[2]);
		poly_t *clone;
		cl_assert(poly != NULL);
		cl_assert(clone = clone_poly(poly));

		rc = poly_classify_poly(poly, clone);
		cl_assert_equal_i(rc, COPLANAR);

		free_poly(clone);
		free_poly(poly);
	}
}
