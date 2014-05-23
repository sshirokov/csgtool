#include "clar.h"

#include "vector.h"

void test_utils__distances(void) {
	float3 a = FLOAT3_INIT;
	float3 b = {1.0, 1.0, 0.0};

	float d2 = f3_distance2(a, b);
	float d =  f3_distance(a, b);

	cl_assert_(d2 == 2, "d2 between origin and (1, 1) should be 1^2 + 1^2 = 2");
	cl_assert_((d < 1.5) && (d > 1.4), "distance between origin and (1, 1) should be root(2)");
}
