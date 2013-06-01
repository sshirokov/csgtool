#
# Copyright (c) Vicent Marti. All rights reserved.
#
# This file is part of clar, distributed under the ISC license.
# For full terms see the included COPYING file.
#

#
# Set up the path to the clar sources and to the fixtures directory
#
# The fixture path needs to be an absolute path so it can be used
# even after we have chdir'ed into the test directory while testing.
#
CURRENT_MAKEFILE  := $(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))
TEST_DIRECTORY    := $(abspath $(dir $(CURRENT_MAKEFILE)))
CLAR_PATH         := $(TEST_DIRECTORY)/clar/
CLAR_FIXTURE_PATH := $(TEST_DIRECTORY)/fixtures/

CFLAGS=-g -I../src -I. -I$(CLAR_PATH) -Wall -DCLAR_FIXTURE_PATH=\"$(CLAR_FIXTURE_PATH)\"

.PHONY: clean test

# App sources
SOURCES = $(shell find "$(TEST_DIRECTORY)/../src" -name '*.c')
LIBS = -lm

# Tests
TESTS = $(shell find . -depth 1 -name '*.c')

# list the objects that go into our test
objects = $(CLAR_PATH)main.o $(patsubst %.c,%.o,$(TESTS)) $(patsubst %.c,%.o,$(SOURCES))

# build the test executable itself
clar_test: $(objects) clar.suite $(TEST_DIRECTORY)/clar_test.h  $(CLAR_PATH)clar.c
	$(CC) $(LIBS) $(CFLAGS) -o $@ "$(CLAR_PATH)clar.c" $(objects)

# test object files depend on clar macros
$(objects) : $(CLAR_PATH)clar.h

# build the clar.suite file of test metadata
clar.suite:
	python "$(CLAR_PATH)generate.py" .

# remove all generated files
clean:
	$(RM) -rf $(CLAR_PATH)*.o *.o clar.suite .clarcache clar_test clar_test.dSYM

# run tests
test: clar_test
	./clar_test
