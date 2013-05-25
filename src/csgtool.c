#include <stdio.h>
#include "klist.h"
#include "dbg.h"
#include "stl.h"

#define mp_float3_free(x) free(kl_val(x))
KLIST_INIT(float3, float3*, mp_float3_free)

typedef struct s_poly {
	klist_t(float3) *vertices;
	float3 normal;
	float w;
} poly_t;

poly_t *alloc_poly(void) {
	poly_t *poly = calloc(1, sizeof(poly_t));
	check_mem(poly);
	poly->vertices = kl_init(float3);
	return poly;
error:
	return NULL;
}

void free_poly(poly_t *p) {
	kl_destroy(float3, p->vertices);
}

#define mp_poly_free(x) free(kl_val(x))
KLIST_INIT(poly, poly_t*, mp_poly_free)

// TODO: move
float3 *f3_cross(float3 *result, float3 v1, float3 v2) {
		float3 v1_x_v2 = {
				v1[1]*v2[2] - v1[2]*v2[1],
				v1[2]*v2[0] - v1[0]*v2[2],
				v1[0]*v2[1] - v1[1]*v2[0]
		};
		memcpy(result, &v1_x_v2, sizeof(float3));
		return result;
}

float3 *f3_normalize(float3 *v) {
	float mag = sqrt((*v)[0] * (*v)[0] +
					 (*v)[1] * (*v)[1] +
					 (*v)[2] * (*v)[2]);
	(*v)[0] /= mag;
	(*v)[1] /= mag;
	(*v)[2] /= mag;
	return v;
}

float3 *f3_sub(float3 *result, float3 v1, float3 v2) {
	float3 r = {v1[0] - v2[0], v1[1] - v2[1], v1[2] - v2[2]};
	memcpy(result, r, sizeof(float3));
	return result;
}

int poly_update(poly_t *poly) {
	check(poly->vertices->size > 2,
		  "poly_update(Polyon(%p)): has %zd verticies.",
		  poly, poly->vertices->size);

	kliter_t(float3) *v_iter = kl_begin(poly->vertices);
	float3 *a = kl_val(v_iter);
	float3 *b = kl_val(v_iter = kl_next(v_iter));
	float3 *c = kl_val(v_iter = kl_next(v_iter));

	float3 b_a;
	float3 c_a;
	f3_sub(&b_a, *b, *a);
	f3_sub(&c_a, *c, *a);

	f3_cross(&poly->normal, b_a, c_a);
	f3_normalize(&poly->normal);
	return 0;
error:
	return -1;
}

int main(int argc, char **argv) {
	check(argc >= 2, "Need a filename");
	char *file = argv[1];
	stl_object *file_stl = stl_read_file(file, 0);
	check(file_stl != NULL, "Failed to read stl from '%s'", file);
	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);

	klist_t(poly) *polygons = kl_init(poly);
	for(int i = 0; i < file_stl->facet_count; i++) {
		stl_facet *face = &file_stl->facets[i];
		poly_t *poly = NULL;

		check(poly = alloc_poly(), "Failed to allocate polygon %d", i);
		*kl_pushp(poly, polygons) = poly;

		// Copy each vertex, using a fresh pointer
		// and letting the poly deallocator deal with it
		float3 *f = NULL;
		for(int v = 0; v < 3; v++) {
			check_mem(f = malloc(sizeof(float3)));
			memcpy(f, face->vertices[v], sizeof(float3));
			*kl_pushp(float3, poly->vertices) = f;
		}
		poly_update(poly);

		printf("Adding %d/%d\r", i+1, file_stl->facet_count);
	}
	putchar('\n');

	kl_destroy(poly, polygons);
	stl_free(file_stl);
	log_info("Terminating Success");
	return 0;
error:
	kl_destroy(poly, polygons);
	if(file_stl != NULL) stl_free(file_stl);
	log_err("Terminating Failure");
	return -1;
}
