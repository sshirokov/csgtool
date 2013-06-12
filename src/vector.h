#include <string.h>
#include <math.h>

#include "klist.h"

#ifndef __VECTOR_H
#define __VECTOR_H

// Wrappers
typedef float float4[4];
#define FLOAT4_INIT {0.0, 0.0, 0.0, 0.0}
#define FLOAT4_INIT_MAX {INFINITY, INFINITY, INFINITY, INFINITY}
#define FLOAT4_INIT_MIN {-INFINITY, -INFINITY, -INFINITY, -INFINITY}
#define FLOAT4_FORMAT(x) (x)[0], (x)[1], (x)[2], (x)[3]
#define f4X(x) (x)[0]
#define f4Y(x) (x)[1]
#define f4Z(x) (x)[2]
#define f4W(x) (x)[3]

float4 *clone_f3(float4 f);

// Vector Updating operations
float4 *f4_normalize(float4 *v);
float4 *f4_scale(float4 *f, float c);

// Non-destructive
float4 *f4_cross(float4 *result, float4 v1, float4 v2);
float f4_dot(float4 v1, float4 v2);
float4 *f4_sub(float4 *result, float4 v1, float4 v2);
float4 *f4_interpolate(float4 *result, float4 start, float4 v, float alpha);

// Containers
#define mp_float4_free(x) free(kl_val(x))
KLIST_INIT(float4, float4*, mp_float4_free)

#endif
