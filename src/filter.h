#include "klist.h"
#include "poly.h"
#include "index.h"

#ifndef __FILTER_H
#define __FILTER_H

// Filter and map signatures
typedef int (filter_test_t)(poly_t *poly);
typedef void (poly_with_index_t)(klist_t(poly) *dst, mesh_index_t *index, poly_t *poly);

// Filters
int filter_test_edge_singularity(poly_t *poly);

// Maps
void map_bisect_edges(klist_t(poly) *dst, mesh_index_t *index, poly_t *poly);

// Filter driver
klist_t(poly) *filter_polys(klist_t(poly) *dst, klist_t(poly) *src, filter_test_t *test);

// Map driver
klist_t(poly) *map_polys_with_index(mesh_index_t *idx, klist_t(poly) *dst, klist_t(poly) *src, poly_with_index_t *mapper);

#endif
