#-*- mode:makefile-gmake; -*-
ROOT = $(shell pwd)
TARGET = csgtool

INCLUDE += -I$(ROOT)/src
SOURCES = $(wildcard $(ROOT)/src/*.c)

OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
LIBS = -lm -lcsg $(OPTLIBS)
CFLAGS = -g -std=c99 $(INCLUDE) -Wall -Werror $(OPTFLAGS)

LIB_TARGET = libcsg.a

ifeq ($(shell uname),Linux)
LIBTOOL_FLAGS = --mode=compile
endif

.DEFAULT_GOAL = all
all: $(LIB_TARGET) $(TARGET)

clean:
	make -C tests clean
	rm -rf $(OBJS) $(TARGET) $(TARGET).o $(TARGET).new

test:
	@make -C tests clean test

.PHONY: all clean test

$(TARGET): $(OBJS) $(TARGET).o
	$(CC) $(CFLAGS) $^ -L. $(LIBS) -o $@.new
	mv $@.new $@

$(LIB_TARGET): $(OBJS)
	libtool $(LIBTOOL_FLAGS) -static -o $(LIB_TARGET) - $(OBJS)

%.o: %.c
	$(CC) -fPIC $(CFLAGS) -o $@ -c $^
