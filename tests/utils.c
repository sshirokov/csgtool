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
// [x] line_with_nl
// [x] empty
// [x] line_without_nl
// [x] multiline_text
// multiline_text_no_nl

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
	char *data = read_line(fp, true, true);
	cl_assert_equal_i(strcmp("i am a cat", data), 0);

	// Cleanup locals
	if(data != NULL) free(data);
}

void test_utils__read_line_doesnt_vomit_on_empty_file(void) {
	fp = fopen(CLAR_FIXTURE_PATH "empty", "r");

	char *data = read_line(fp, true, true);
	cl_assert_(data == NULL, "Should have read NULL from an empty file.");

	// Clean up anything we shouldn't have
	if(data != NULL) free(data);
}

void test_utils__read_line_reads_without_nl(void) {
	fp = fopen(CLAR_FIXTURE_PATH "line_without_nl", "r");

	char *data = read_line(fp, false, true);
	cl_assert_equal_i(strcmp("I am a cat", data), 0);

	if(data != NULL) free(data);
}

void test_utils__read_line_by_line_from_multiline_with_nl(void) {
	fp = fopen(CLAR_FIXTURE_PATH "multiline_text", "r");

	char *data = NULL;

	data = read_line(fp, false, true);
	cl_assert_equal_i(strcmp("I am a cat", data), 0);
	if(data != NULL) free(data);

	data = read_line(fp, true, true);
	cl_assert_equal_i(strcmp("we are all cats", data), 0);
	if(data != NULL) free(data);

	cl_assert_(read_line(fp, true, true) == NULL,
			   "Should get a NULL after we run out of file.");
}

void test_utils__read_line_by_line_from_multiline_without_nl(void) {
	fp = fopen(CLAR_FIXTURE_PATH "multiline_text_no_nl", "r");

	char *data = NULL;

	data = read_line(fp, false, true);
	cl_assert_equal_i(strcmp("I am a cat", data), 0);
	if(data != NULL) free(data);

	data = read_line(fp, true, true);
	cl_assert_equal_i(strcmp("we are all cats", data), 0);
	if(data != NULL) free(data);

	cl_assert_(read_line(fp, true, true) == NULL,
			   "Should get a NULL after we run out of file.");
}
