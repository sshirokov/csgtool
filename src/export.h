#include <string.h>

#include "dbg.h"

#include "stl.h"
#include "poly.h"
#include "bsp.h"

#ifndef __EXPORT_H
#define __EXPORT_H

stl_object *stl_from_polys(klist_t(poly) *polygons);
stl_object *bsp_to_stl(bsp_node_t *tree);

#endif
