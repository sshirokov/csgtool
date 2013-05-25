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


int main(int argc, char **argv) {
	check(argc >= 2, "Need a filename");
	char *file = argv[1];
	stl_object *file_stl = stl_read_file(file);
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
