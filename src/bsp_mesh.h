
#ifndef __BSP_MESH_H
#define __BSP_MESH_H

extern mesh_t bsp_mesh_t_Proto;

typedef struct s_bsp_mesh_t {
	mesh_t proto;
	bsp_node_t *bsp;
} bsp_mesh_t;

#endif
