#include "stl.h"
#include "mesh.h"

#ifndef __STL_MESH_H
#define __STL_MESH_H

// mesh_t type and prototype
extern mesh_t stl_mesh_t_Proto;

typedef struct s_stl_mesh_t {
	mesh_t proto;
	stl_object *stl;
} stl_mesh_t;

// Alternative implementations for `to_polygons` are available.
// The default performs sanity checks on the constructed polygons,
// the `_unsafe` variant ignores these checks and returns a polygon
// list as-defined, but possibly mathematically useless.
klist_t(poly)* stl_mesh_to_polygons(void *self);
klist_t(poly)* stl_mesh_to_polygons_unsafe(void *self);

#endif
