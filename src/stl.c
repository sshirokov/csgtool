#include <ctype.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"

#include "stl.h"
#include "util.h"

void stl_free(stl_object *obj) {
		if(obj == NULL) return;
		if(obj->facets) free(obj->facets);
		free(obj);
}

stl_object *stl_alloc(char *header, uint32_t n_facets) {
		stl_object *obj = (stl_object*)calloc(1, sizeof(stl_object));
		assert_mem(obj);

		if(header != NULL) {
				memcpy(obj->header, header, sizeof(obj->header));
		}

		obj->facet_count = n_facets;
		if(n_facets > 0) {
				obj->facets = (stl_facet*)calloc(n_facets, sizeof(stl_facet));
				assert_mem(obj->facets);
		}

		return obj;
}

void v3_cross(float3 *result, float3 v1, float3 v2, int normalize) {
		float3 v1_x_v2 = {
				v1[1]*v2[2] - v1[2]*v2[1],
				v1[2]*v2[0] - v1[0]*v2[2],
				v1[0]*v2[1] - v1[1]*v2[0]
		};
		if(normalize) {
				f3_normalize(&v1_x_v2);
		}
		memcpy(result, &v1_x_v2, sizeof(float3));
}

void stl_facet_update_normal(stl_facet *facet) {
		float3 *fvs = facet->vertices;
		float3 v1 = {fvs[0][0] - fvs[1][0], fvs[0][1] - fvs[1][1], fvs[0][2] - fvs[1][2]};
		float3 v2 = {fvs[0][0] - fvs[2][0], fvs[0][1] - fvs[2][1], fvs[0][2] - fvs[2][2]};
		v3_cross(&facet->normal, v1, v2, 1);
}

stl_facet *stl_read_facet(int fd) {
		int rc = -1;
		stl_facet *facet = (stl_facet*)calloc(1, sizeof(stl_facet));
		assert_mem(facet);

		rc = read(fd, &facet->normal, sizeof(facet->normal));
		check(rc == sizeof(facet->normal), "Failed to read normal. Read %d expected %zu", rc, sizeof(facet->normal));
		rc = read(fd, &facet->vertices, sizeof(facet->vertices));
		check(rc == sizeof(facet->vertices), "Failed to read triangle vertecies. Read %d expected %zu", rc, sizeof(facet->vertices));
		rc = read(fd, &facet->attr, sizeof(facet->attr));
		check(rc == sizeof(facet->attr), "Failed to read attr bytes. Read %d expect %zu", rc, sizeof(facet->attr));

		return facet;
error:
		exit(-1);
}

stl_facet *stl_read_text_facet(const char *declaration, FILE *f) {
		stl_facet *facet = (stl_facet*)calloc(1, sizeof(stl_facet));
		int rc = -1;
		char *line = NULL;

		assert_mem(facet);
		rc = sscanf(declaration, "facet normal %f %f %f", &facet->normal[0], &facet->normal[1], &facet->normal[2]);
		check(rc == 3, "stl_Read_text_facet(%s): Normal line malformed", declaration);

		check((line = next_line(f, true, true)), "Malformed facet, no line follows valid normal declaration");
		check(strcmp(line, "outer loop") == 0, "Malformed facet, no loop declaration following valid normal.");

		for(int i = 0; i < 3; i++) {
				check((line = next_line(f, true, true)), "Failed to read vertex %d", i);
				rc = sscanf(line, "vertex %f %f %f", &facet->vertices[i][0], &facet->vertices[i][1], &facet->vertices[i][2]);
				check(rc == 3, "Vertex declaration [%s] did not contain (x, y, z) point.", line);

				free(line);
				line = NULL;
		}

		check((line = next_line(f, true, true)), "No line following vertex declarations.");
		check(strcmp(line, "endloop") == 0, "Vertex declarations not followed by 'endloop'. Got: '%s'", line);
		free(line);
		check((line = next_line(f, true, true)), "No line following endloop.");
		check(strcmp(line, "endfacet") == 0, "endloop not followed by 'endfacet'");
		free(line);
		line = NULL;

		return facet;
error:
		if(line) free(line);
		if(facet) free(facet);
		return NULL;
}

stl_object *stl_read_text_object(int fd) {
	FILE *f = NULL;
	stl_object *obj = NULL;
	char *line = NULL;
	klist_t(stl_facet) *facets = NULL;

	check((f = fdopen(fd, "r")) != NULL, "Failed to open fd(%d) as FILE", fd);
	check((facets = kl_init(stl_facet)) != NULL, "Failed to create facel list.");

	// Read first non-blank-line
	line = next_line(f, false, true);

	check(line != NULL, "Failed to read STL/ASCII header.");
	check((obj = stl_alloc(NULL, 0)), "Failed to allocated new STL object.");
	snprintf(obj->header, sizeof(obj->header), "[STL/ASCII]: '%s'", line);
	// debug("Header: [%s]", obj->header);
	free(line);

	size_t lines = 0;
	while((line = read_line(f, true, true))) {
		lines++;

		// Skip blanks
		if(strlen(line) == 0) {
			free(line);
			continue;
		}

		if(strncmp(line, "facet", strlen("facet")) == 0) {
			stl_facet *facet = stl_read_text_facet(line, f);
			check(facet != NULL, "Failed to read facet on line %zd", lines);
			*kl_pushp(stl_facet, facets) = facet;
		}
		else if(strncmp(line, "endsolid", strlen("endfacet")) == 0) {
			check(facets->size > 0, "No facets loaded.");
			// debug("ASCII solid ended. Loaded %zd facets", facets->size);

			obj->facet_count = facets->size;
			obj->facets = calloc(facets->size, sizeof(stl_facet));
			assert_mem(obj->facets);

			stl_facet *facet = NULL;
			for(int i = 0; kl_shift(stl_facet, facets, &facet) != -1; i++) {
				obj->facets[i] = *facet;
			}
		}
		else {
			sentinel("Unexpected line[%zd]: '%s'", lines, line);
		}
		free(line);
	}

	kl_destroy(stl_facet, facets);
	return obj;
error:
	kl_destroy(stl_facet, facets);
	return NULL;
}

stl_object *stl_read_object(int fd) {
		int rc = -1;
		char header[80] = {0};
		uint32_t n_tris = 0;
		stl_object *obj = NULL;

		// Read the header
		rc = read(fd, header, sizeof(header));
		check(rc == sizeof(header), "Unable to read STL header. Got %d bytes.", rc);

		// Triangle count
		rc = read(fd, &n_tris, sizeof(n_tris));
		check(rc == sizeof(n_tris), "Failed to read facet count.");

		obj = stl_alloc(header, n_tris);

		for(uint32_t i = 0; i < obj->facet_count; i++) {
				stl_facet *facet = stl_read_facet(fd);
				memcpy(&obj->facets[i], facet, sizeof(stl_facet));
				free(facet);
		}

		return obj;
error:
		if(obj) stl_free(obj);
		return NULL;
}

stl_object *stl_read_file(char *path, int recompute_normals) {
		stl_reader* reader = NULL;
		stl_object *obj = NULL;
		int fd = -1;

		check((reader = stl_detect_reader(path)), "Unable to find reader for format of %s", path);
		check((fd = open(path, O_RDONLY)) != -1, "Unable to open '%s'", path);

		obj = reader(fd);
		check(obj != NULL, "Failed to read STL from %s", path);

		char buffer[10];
		int rc = read(fd, buffer, sizeof(buffer));
		check(rc == 0, "File did not end when expected, assuming failure.");
		close(fd);

		if(recompute_normals) {
			for(int i = 0; i < obj->facet_count; i++) {
				stl_facet_update_normal(&obj->facets[i]);
			}
		}

		return obj;
error:
		if(fd != -1) close(fd);
		if(obj) stl_free(obj);
		return NULL;
}

int stl_write_file(stl_object *obj, char *path) {
		int rc = -1;
		int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
		check(fd != -1, "Failed to open '%s' for write", path);

		rc = stl_write_object(obj, fd);

		close(fd);
		return rc;
error:
		if(fd != -1) close(fd);
		return -1;
}

int stl_write_object(stl_object *obj, int fd) {
		int rc = -1;

		rc = write(fd, obj->header, sizeof(obj->header));
		check(rc == sizeof(obj->header), "Failed to write object header");

		rc = write(fd, &obj->facet_count, sizeof(obj->facet_count));
		check(rc == sizeof(obj->facet_count), "Failed to write face count");

		for(uint32_t i = 0; i < obj->facet_count; i++) {
				rc = stl_write_facet(&obj->facets[i], fd);
				check(rc == 0, "Failed to write facet %d", i);
		}

		return 0;
error:
		return -1;
}

int stl_write_facet(stl_facet *facet, int fd) {
		// Pre-computed size since sizeof(struct) falls to padding problems for IO
		const size_t facet_size = sizeof(facet->normal) + sizeof(facet->vertices) + sizeof(facet->attr);
		int rc = -1;

		rc = write(fd, facet, facet_size);
		check(rc == facet_size, "Failed to write facet struct");

		return 0;
error:
		return -1;
}

stl_reader* stl_detect_reader(char *path) {
		stl_reader* reader = stl_read_text_object;
		static const int upto = 100;
		char c = '\0';
		int rc = -1;
		int fd = open(path, O_RDONLY);
		check(fd != -1, "Failed to open %s for format detection.", path);
		for(int i = 0; i < upto; i++) {
				check((rc = read(fd, &c, 1)) == 1, "Failed to read byte %d for reader detection of %s", i, path);
				if(!isprint(c) && !isspace(c)) {
						reader = stl_read_object;
						break;
				}
		}

		if(fd != -1) close(fd);
		return reader;
error:
		if(fd != -1) close(fd);
		return NULL;
}
