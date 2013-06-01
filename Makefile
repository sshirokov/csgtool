#-*- mode:makefile-gmake; -*-
ROOT = $(shell pwd)
TARGET = csgtool

INCLUDE += -I$(ROOT)/src
SOURCES = $(shell find $(ROOT)/src -name '*.c')

OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
CFLAGS = -g $(INCLUDE) -Wall -Werror $(OPTFLAGS)
LIBS = -lm $(OPTLIBS)

.DEFAULT_GOAL = all
all: $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET) $(TARGET).o $(TARGET).new

test:
	@make -C tests clean test

.PHONY: all clean test

$(TARGET): $(OBJS) $(TARGET).o
	$(CC) $(CFLAGS) -o $@.new $(LIBS) $^
	mv $@.new $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
