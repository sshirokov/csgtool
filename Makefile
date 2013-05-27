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
	rm -rf $(OBJS) $(TARGET) $(TARGET).new

.PHONY: all clean

$(TARGET): $(OBJS) $(TARGET).o
	$(CC) $(CFLAGS) -o $@.new $(LIBS) $^
	mv $@.new $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
