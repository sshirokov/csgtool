#-*- mode:makefile-gmake; -*-
ROOT = $(shell pwd)
TARGET = csgtool

INCLUDE += -I$(ROOT)/src
SOURCES = $(wildcard $(ROOT)/src/*.c)

OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
LIBS = -lm $(OPTLIBS)
LIBS_posix = -lm
CFLAGS = -g -std=c99 $(INCLUDE) -Wall -Werror $(OPTFLAGS) -lm

.DEFAULT_GOAL = all
all: $(TARGET)

clean:
	make -C tests clean
	rm -rf $(OBJS) $(TARGET) $(TARGET).o $(TARGET).new

test:
	@make -C tests clean test

.PHONY: all clean test

$(TARGET): $(OBJS) $(TARGET).o
	$(CC) $(CFLAGS) -o $@.new $(LIBS) $^
	mv $@.new $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ $(LIBS) -c $^
