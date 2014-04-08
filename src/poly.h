#include <stdbool.h>
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

#ifndef POLY_MAX_VERTS
#define POLY_MAX_VERTS 40
#endif

typedef struct s_poly {
	float3 *vertices;
	int vertex_count;
	int vertex_max;

	float3 normal;
	float w;

	float3 _vbuffer[POLY_MAX_VERTS];
} poly_t;

poly_t *alloc_poly(void);
poly_t *poly_make_triangle(float3 a, float3 b, float3 c);
poly_t *clone_poly(poly_t *poly);
void free_poly(poly_t *p, int free_self);

poly_t *poly_init(poly_t *poly);
int poly_update(poly_t *poly);
poly_t *poly_invert(poly_t *poly);

bool poly_has_area(poly_t *poly);

int poly_vertex_count(poly_t *poly);
int poly_vertex_max(poly_t *poly);
int poly_vertex_available(poly_t *poly);
int poly_vertex_dynamic_p(poly_t *poly);
int poly_vertex_expand(poly_t *poly);
int poly_push_vertex(poly_t *poly, float3 v);

int poly_classify_vertex(poly_t *poly, float3 v);
int poly_classify_poly(poly_t *this, poly_t *other);

int poly_split(poly_t *divider, poly_t *poly, poly_t **front, poly_t **back);

#define mp_poly_free(x) free_poly(kl_val(x), 1)
KLIST_INIT(poly, poly_t*, mp_poly_free)

#endif
