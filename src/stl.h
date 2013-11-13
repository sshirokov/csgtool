#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include "klist.h"
#include "vector.h"
#include "mesh.h"

#ifndef __STL_H
#define __STL_H

// File format structs
typedef struct s_stl_facet {
		float3 normal;
		float3 vertices[3];
		uint16_t attr;
} stl_facet;

typedef struct s_stl_object {
		char header[80];
		uint32_t facet_count;
		stl_facet *facets;
} stl_object;


// Alloc/Free
void stl_free(stl_object *obj);
stl_object *stl_alloc(char *header, uint32_t n_facets);

// Operations
void stl_facet_update_normal(stl_facet *facet);

// Signatures of file readers
typedef stl_object* (stl_reader)(int fd);

// File reader factory and dispatcher
stl_reader* stl_detect_reader(char *path);
stl_object *stl_read_file(char *path, int recompute_normals);

// Binary file readers
stl_facet *stl_read_facet(int fd);
stl_object *stl_read_object(int fd);

// Text file readers
stl_facet *stl_read_text_facet(const char *declaration, int fd);
stl_object *stl_read_text_object(int fd);

// Binary file writers
int stl_write_file(stl_object *obj, char *path);
int stl_write_object(stl_object *obj, int fd);
int stl_write_facet(stl_facet *facet, int fd);

// Composite wrappers
#define mp_stl_free(x) stl_free(kl_val(x))
KLIST_INIT(stl_object, stl_object*, mp_stl_free)
#define mp_std_free(x) free(kl_val(x))
KLIST_INIT(stl_facet, stl_facet*, mp_std_free)

// mesh_t type and prototype
extern mesh_t stl_mesh_t_Proto;

typedef struct s_stl_mesh_t {
	mesh_t proto;
	stl_object *stl;
} stl_mesh_t;

int stl_mesh_init(void *self);
void stl_mesh_destroy(void *self);
int stl_mesh_poly_count(void *self);
klist_t(poly)* stl_mesh_to_polygons(void *self);



#endif
