#include <strings.h>
#include "dbg.h"
#include "klist.h"
#include "vector.h"

#ifndef __POLY_H
#define __POLY_H

#define EPSILON 1e-5
#define COPLANAR 0
#define FRONT 1
#define BACK 2
#define SPANNING 3

typedef struct s_poly {
	klist_t(float3) *vertices;
	float3 normal;
	float w;
} poly_t;

poly_t *alloc_poly(void);
poly_t *clone_poly(poly_t *poly);
void free_poly(poly_t *p);

poly_t *poly_init(poly_t *poly);
int poly_update(poly_t *poly);

int poly_classify_vertex(poly_t *poly, float3 v);
int poly_classify_poly(poly_t *this, poly_t *other);

poly_t *poly_split(poly_t *divider, poly_t *poly);

#define mp_poly_free(x) free(kl_val(x))
KLIST_INIT(poly, poly_t*, mp_poly_free)

#endif
