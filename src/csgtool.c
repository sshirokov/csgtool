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

int main(int argc, char **argv) {
	check(argc >= 2, "Need a filename");
	char *file = argv[1];
	stl_object *file_stl = stl_read_file(file);
	check(file_stl != NULL, "Failed to read stl from '%s'", file);

	log_info("Loaded file: %s %d facets", file, file_stl->facet_count);


	stl_free(file_stl);
	return 0;
error:
	if(file_stl != NULL) stl_free(file_stl);
	return -1;
}
