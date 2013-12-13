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

#endif
