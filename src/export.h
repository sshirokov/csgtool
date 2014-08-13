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
mesh_t* bsp_to_mesh(bsp_node_t *tree, int copy);
klist_t(poly) *poly_to_tris(klist_t(poly)* dst, poly_t *src);
klist_t(poly)* polys_to_tris(klist_t(poly)* dst, klist_t(poly)* src);

#endif
