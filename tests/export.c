#include "clar.h"

#include "stl.h"
#include "bsp.h"

char jaws_path[] = CLAR_FIXTURE_PATH "jaws.stl";
stl_object *jaws_object = NULL;
bsp_node_t *jaws_bsp = NULL;

void test_export__initialize(void) {

}

void test_export__cleanup(void) {
	if(jaws_object) stl_free(jaws_object);
	// TODO: free_bsp()
}
