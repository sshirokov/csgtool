#include "clar.h"

#include "vector.h"

float3 z = {0.0, 0.0, 0.0};
float3 a = {1.0, 1.0, 1.0};
float3 b = {-1.0, -1.0, -1.0};

void test_vector__comparison(void) {
	cl_assert_equal_i(f3_cmp(a, a), 0);
	cl_assert_equal_i(f3_cmp(z, a), -1);
	cl_assert_equal_i(f3_cmp(z, b), 1);
	cl_assert_equal_i(f3_cmp(b, a), -1);
	cl_assert_equal_i(f3_cmp(a, b), 1);
}
