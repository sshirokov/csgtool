#-*- mode:makefile-gmake; -*-
ROOT = $(shell pwd)
TARGET = csgtool

INCLUDE += -I$(ROOT)/src
SOURCES = $(wildcard $(ROOT)/src/*.c)

OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
LIBS = -lm $(OPTLIBS)
CFLAGS = -D_POSIX_C_SOURCE=200112L -g -std=c99 $(INCLUDE) -Wall -Werror $(OPTFLAGS)

ifeq ($(shell uname),Darwin)
LIB_TARGET = libcsg.dylib
else
LIB_TARGET = libcsg.so
endif

.DEFAULT_GOAL = all
all: $(TARGET) $(LIB_TARGET)

clean:
	make -C tests clean
	rm -rf $(OBJS) $(TARGET) $(TARGET).o $(TARGET).new $(LIB_TARGET)

test:
	@make -C tests clean test

.PHONY: all clean test libcsg loc

$(TARGET): $(TARGET).o $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@.new
	mv $@.new $@

$(LIB_TARGET): $(OBJS)
	$(CC) -shared $(OBJS) $(LIBS) -o $(LIB_TARGET)

libcsg: $(LIB_TARGET)

%.o: %.c
	$(CC) -fPIC $(CFLAGS) -o $@ -c $^

loc:
	@echo "=> Source:"
	find src/ -name '*.[ch]' -not -name 'dbg.*' -not -name 'klist.*' | xargs wc -l csgtool.c
	@echo "=> Tests:"
	find tests/ -name '*.[ch]' -not -path '*clar*' | xargs wc -l
