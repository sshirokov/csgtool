#include "vector.h"

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
