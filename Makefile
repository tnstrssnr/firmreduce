SHELL=/bin/sh
CC = gcc
LINK ?= $(CC)
DLL_EXT ?= .so

top_src_dir = .
top_build_dir = build
vpath %.c $(top_src_dir)/src
vpath %.h $(top_src_dir)/include

variant = debug
build_dir = $(top_build_dir)/$(variant)

# set compile flags
CFLAGS += -Wall -g

# set link flags
LINK_FLAGS += $(FIRM_LIBS)

passes_SOURCES = $(wildcard src/passes/*.c)
passes_OBJECTS = $(passes_SOURCES:%.c=%(build_dir)/%.o)
passes_STATIC = $(builddir)/firmreduce_passes.a
passes_DYNAMIC = $(builddir)/firmreduce_passes.$(DLL_EXT)

firmreduce_SOURCES = main.c $(passes_SOURCES)
firmreduce_OBJECTS = $(firmreduce_SOURCES:%.c=%(build_dir)/%.o)
firmreduce_DEPENDS = $(firmreduce_OBJECTS:%.o=%.d)
firmreduce_TARGET = $(build_dir)/firmreduce

libfirm_HOME = $(top_src_dir)/libfirm
libfirm_INCLUDE_FLAGS = -I$(libfirm_HOME)/include -I$(libfirm_HOME)/include/libfirm -I$(libfirm_HOME)/build/gen/include/libfirm
libfirm_STATIC = $(libfirm_HOME)/build/$(variant)/libfirm.a
libfirm_STATIC_PATH = -L$(libfirm_HOME)/build/$(variant)/
libfirm_DYNAMIC = $(libfirm_HOME)/build/$(variant)/libfirm.$(DLL_EXT)

# include headers
SYSTEM_INCLUDE ?= /usr/include
LOCAL_INCLUDE ?= /usr/local/include


# clear implicit suffix rules
.SUFFIXES:

.PHONY: all clean

all: $(firmreduce_TARGET)
	@echo All done

$(firmreduce_TARGET): $(firmreduce_SOURCES) $(libfirm_STATIC)
	LD_LIBRARY_PATH=$(libfirm_STATIC_PATH) $(CC) $(CFLAGS) -o $@ $< $(libfirm_INCLUDE_FLAGS) $(libfirm_STATIC_PATH) -lfirm

clean:
	rm -rf $(build_dir)/*
	@echo Clean done
