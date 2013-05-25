#include <unistd.h>
#include <stdint.h>
#include <math.h>

#include "klist.h"

#ifndef __STL_H
#define __STL_H

// Wrappers
typedef float float3[3];
#define FLOAT3_INIT {0.0, 0.0, 0.0}
#define FLOAT3_INIT_MAX {INFINITY, INFINITY, INFINITY}
#define FLOAT3_INIT_MIN {-INFINITY, -INFINITY, -INFINITY}
#define FLOAT3_FORMAT(x) (x)[0], (x)[1], (x)[2]
#define f3X(x) (x)[0]
#define f3Y(x) (x)[1]
#define f3Z(x) (x)[2]

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
stl_object *stl_read_file(char *path);

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

#endif
