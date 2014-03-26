#include "clar.h"

#include "util.h"

FILE *fp = NULL;

void test_utils__initialize(void) {

}

void test_utils__cleanup(void) {
	// Close anything we left open
	if(fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
}

// TODO: Test fixtures, rooted at CLAR_FIXTURE_PATH
// each one should be their own test, p-much
//
// line_with_nl
// line_without_nl
// multiline_text
// multiline_text_no_nl
// empty

void test_utils__read_line_can_read_a_line(void) {
	fp = fopen(CLAR_FIXTURE_PATH "line_with_nl", "r");

	char *data = read_line(fp, false, false);
	cl_assert_equal_i(strcmp("I am a cat\n", data), 0);

	// Cleanup locals
	if(data != NULL) free(data);
}

void test_utils__read_line_can_read_a_line_and_trim(void) {
	fp = fopen(CLAR_FIXTURE_PATH "line_with_nl", "r");

	// Note the lack of '\n'
	char *data = read_line(fp, false, true);
	cl_assert_equal_i(strcmp("I am a cat", data), 0);

	// Cleanup locals
	if(data != NULL) free(data);
}

void test_utils__read_line_can_read_a_line_and_downcase(void) {
	fp = fopen(CLAR_FIXTURE_PATH "line_with_nl", "r");

	// Note the lack of '\n'
	char *data = read_line(fp, true, false);
	cl_assert_equal_i(strcmp("i am a cat\n", data), 0);

	// Cleanup locals
	if(data != NULL) free(data);
}

void test_utils__read_line_can_read_a_line_then_trim_and_downcase(void) {
	fp = fopen(CLAR_FIXTURE_PATH "line_with_nl", "r");

	// Note the lack of '\n'
	char *data = read_line(fp, true, false);
	cl_assert_equal_i(strcmp("i am a cat", data), 0);

	// Cleanup locals
	if(data != NULL) free(data);
}
