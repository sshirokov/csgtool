#include <string.h>

#include "dbg.h"

#include "stl.h"
#include "poly.h"
#include "bsp.h"
#include "mesh.h"

#ifndef __EXPORT_H
#define __EXPORT_H

stl_object *stl_from_polys(klist_t(poly) *polygons);
stl_object *bsp_to_stl(bsp_node_t *tree);
bsp_node_t *stl_to_bsp(stl_object *stl);
bsp_node_t *mesh_to_bsp(mesh_t *mesh);

#endif
