#include "cmd_audit.h"

// The finalizer for `cmd_audit`. Cleans up the structures
// used, and then returns the value passed.
// Because I fuckign hate spelling this shit out twice, like I
// do literally everywhere else in this codebase.
#define __FINISH_CMD_AUDIT(ret) do {				\
		if(in != NULL) free_mesh(in);				\
		if(polys != NULL) kl_destroy(poly, polys);	\
	} while(false);									\
	return (ret)

// `audit` command entry point.
// Takes one mesh as the last arg then walks it looking for
// abnormalities.
int cmd_audit(int argc, char *argv[]) {
	char *name = argv[argc - 1];
	mesh_t *in = NULL;
	klist_t(poly)* polys = NULL;

	check(argc >= 1, "Too few args");
	check(in = mesh_read_file(name), "Failed to read [%s]", name);
	check(in->poly_count(in) > 0, "Mesh does not contain any polygons.");

	// If we load an STL, we need to patch mesh->to_polygons(..) to
	// a version that bypasses the `poly_push_vertex` checks, otherwise
	// invalid polygons simply won't be created, and a shitload of warning
	// spam will appear.
	const char *stl_type = "STL";
	if(strncmp(in->type, stl_type, strlen(stl_type)) == 0) {
		log_info("Patching mesh to produce unsafe polygons.");
		// TODO: Patch in a new method
		in->to_polygons = in->to_polygons;
	}


	log_info("Converting mesh to polygon list for walk.");
	check(polys = in->to_polygons(in), "Failed to get polygons from mesh.");

	log_info("Loaded [%d] polys from '%s', beginning walk", in->poly_count(in), name);

	kliter_t(poly) *iter = kl_begin(polys);
	poly_t *poly = NULL;
	size_t count = 0;
	size_t bad_count = 0;
	for(; iter != kl_end(polys); iter = kl_next(iter), count++) {
		poly = kl_val(iter);
		if(poly == NULL) {
			log_warn("Failed to get polygon %zd from mesh, it is NULL", count);
			bad_count++;
			continue;
		}

		// If a squard edge length is zero, it's zero.
		if(poly_min_edge_length2(poly) == 0.0) {
			bad_count++;
			log_warn("Poly %zd has an edge of length2 = %f", count, poly_min_edge_length2(poly));
			poly_print(poly, stderr);
		}
	}

	log_info("Checked %zd polygons. %zd had problems.", count, bad_count);

	__FINISH_CMD_AUDIT(bad_count);
error:
	__FINISH_CMD_AUDIT(-1);
}
