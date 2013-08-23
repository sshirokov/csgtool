#include <string.h>
#include <math.h>

#include "klist.h"

#ifndef __VECTOR_H
#define __VECTOR_H

// Wrappers
typedef float float3[3];
#define FLOAT3_INIT {0.0, 0.0, 0.0}
#define FLOAT3_INIT_MAX {INFINITY, INFINITY, INFINITY}
#define FLOAT3_INIT_MIN {-INFINITY, -INFINITY, -INFINITY}
#define FLOAT3_FORMAT(x) (x)[0], (x)[1], (x)[2]
#define f3X(x) (x)[0]
#define f3Y(x) (x)[1]
#define f3Z(x) (x)[2]

float3 *clone_f3(float3 f);

// Vector Updating operations
float3 *f3_normalize(float3 *v);
float3 *f3_scale(float3 *f, float c);

// Non-destructive
float3 *f3_cross(float3 *result, float3 v1, float3 v2);
float f3_dot(float3 v1, float3 v2);
float3 *f3_sub(float3 *result, float3 v1, float3 v2);
float3 *f3_interpolate(float3 *result, float3 start, float3 v, float alpha);
int f3_cmp(float3 a, float3 b);

// Containers
#define mp_float3_free(x) free(kl_val(x))
KLIST_INIT(float3, float3*, mp_float3_free)

#endif
