#include "vector.h"

float4 *clone_f4(float4 f) {
	float4 *clone = malloc(sizeof(float4));
	if(clone) {
		(*clone)[0] = f[0];
		(*clone)[1] = f[1];
		(*clone)[2] = f[2];
		(*clone)[3] = f[3];
	}
	return clone;
}

float4 *f4_normalize(float4 *v) {
	float mag = sqrt((*v)[0] * (*v)[0] +
					 (*v)[1] * (*v)[1] +
					 (*v)[2] * (*v)[2] +
					 (*v)[3] * (*v)[3]);
	(*v)[0] /= mag;
	(*v)[1] /= mag;
	(*v)[2] /= mag;
	return v;
}

float4 *f4_scale(float4 *v, float c) {
	(*v)[0] *= c;
	(*v)[1] *= c;
	(*v)[2] *= c;
	(*v)[3] *= c;
	return v;
}

float f4_dot(float4 v1, float4 v2) {
	return (v1[0] * v2[0] +
			v1[1] * v2[1] +
			v1[2] * v2[2] +
		    v1[3] * v2[3]);
}

float4 *f4_cross(float4 *result, float4 v1, float4 v2) {
		(*result)[0] = v1[1]*v2[2] - v1[2]*v2[1];
		(*result)[1] = v1[2]*v2[0] - v1[0]*v2[2];
		(*result)[2] = v1[0]*v2[1] - v1[1]*v2[0];
		(*result)[3] = 0;
		return result;
}

float4 *f4_sub(float4 *result, float4 v1, float4 v2) {
	(*result)[0] = v1[0] - v2[0];
	(*result)[1] = v1[1] - v2[1];
	(*result)[2] = v1[2] - v2[2];
	(*result)[3] = v1[3] - v2[3];
	return result;
}

float4 *f4_interpolate(float4 *result, float4 start, float4 v, float alpha) {
	for(int i = 0; i < 4; i++) {
		(*result)[i] += (v[i] - start[i]) * alpha;
	}
	return result;
}
