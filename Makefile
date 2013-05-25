#-*- mode:makefile-gmake; -*-
## Env Setup
ROOT = $(shell pwd)
TARGET = csgtool

INCLUDE += -I$(ROOT)/src
HEADERS = $(shell find $(INCLUDE) -name '*.h')
SOURCES = $(shell find $(ROOT)/src -name '*.c')

OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
CFLAGS = -g -O2 $(INCLUDE) $(OPTFLAGS) -Wall -Werror
LIBS = $(OPTLIBS) -lm

.DEFAULT_GOAL = all
all: $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET) $(TARGET).new

.PHONY: all clean

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@.new $(LIBS) $^
	mv $@.new $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
