#include "klist.h"
#include "poly.h"

#ifndef __FILTER_H
#define __FILTER_H

typedef int (filter_test_t)(poly_t *poly);

klist_t(poly) *filter_polys(klist_t(poly) *dst, klist_t(poly) *src, filter_test_t test);

#endif
