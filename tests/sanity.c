#include "clar.h"

static int *answer;

void test_sanity__initialize(void)
{
    answer = malloc(sizeof(int));
    cl_assert_(answer != NULL, "No memory left?");
    *answer = 42;
}

void test_sanity__cleanup(void)
{
    free(answer);
}

void test_sanity__make_sure_math_still_works(void)
{
    cl_assert_(5 > 3, "Five should probably be greater than three");
    cl_assert_(-5 < 2, "Negative numbers are small, I think");
    cl_assert_(*answer == 42, "The universe is doing OK. And the initializer too.");
}
