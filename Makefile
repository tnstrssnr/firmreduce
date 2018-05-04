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
pass_dir = $(top_build_dir)/passes

# set compile flags
CFLAGS += -Wall -g

# set link flags
LINK_FLAGS += $(FIRM_LIBS)

passes_SOURCES = $(wildcard $(top_src_dir)/src/passes/*.c)
passes_OBJECTS = $(patsubst %.c,%.o,$(passes_SOURCES))
passes_OBJS = $(wildcard $(pass_dir)/obj/*.o)
passes_DLL =$(patsubst %.o,%.so,$(passes_OBJS))

firmreduce_SOURCES = $(top_src_dir)/src/main.c $(top_src_dir)/src/ir_stats.c $(top_src_dir)/src/passes.c
firmreduce_OBJECTS = $(firmreduce_SOURCES:%.c=%(build_dir)/%.o)
firmreduce_DEPENDS = $(firmreduce_OBJECTS:%.o=%.d)
firmreduce_TARGET = $(build_dir)/firmreduce
firmreduce_INCLUDE_FLAGS = -I$(top_src_dir)/include

libfirm_HOME = $(top_src_dir)/libfirm
libfirm_INCLUDE_FLAGS = -I$(libfirm_HOME)/include -I$(libfirm_HOME)/include/libfirm -I$(libfirm_HOME)/build/gen/include/libfirm
libfirm_STATIC = $(libfirm_HOME)/build/$(variant)/libfirm.a
libfirm_STATIC_PATH = -L$(libfirm_HOME)/build/$(variant)/
libfirm_DYNAMIC_PATH = $(libfirm_HOME)/build/$(variant)/
libfirm_DYNAMIC = $(libfirm_HOME)/build/$(variant)/libfirm.$(DLL_EXT)

# include headers
SYSTEM_INCLUDE ?= /usr/include
LOCAL_INCLUDE ?= /usr/local/include


# clear implicit suffix rules
.SUFFIXES:

.PHONY: all clean clean-all makedir passes

all: makedir $(firmreduce_TARGET) passes # FIXME: needs to be built in this order --> unstable!
	@echo Target make all complete

makedir:
	@mkdir -p $(pass_dir)/obj
	@mkdir -p $(pass_dir)/dll
	@mkdir -p $(build_dir)

$(firmreduce_TARGET): $(firmreduce_SOURCES) $(libfirm_STATIC)
	@echo Build firmreduce
	@$(CC) $(CFLAGS) -o $@ $(firmreduce_SOURCES) $(libfirm_INCLUDE_FLAGS) $(firmreduce_INCLUDE_FLAGS) $(libfirm_STATIC_PATH) -Wl,-rpath=$(libfirm_DYNAMIC_PATH) -lfirm -ldl

$(libfirm_STATIC):
	@cd libfirm && make

clean:
	@$(Q)rm -rf $(top_build_dir)/*
	@echo Target make clean done

clean-all: clean
	@cd libfirm && make clean

passes: makedir $(passes_OBJECTS)
	
	
%.o: %.c
	@$(CC) -c $(CFALGS) -fPIC $< $(libfirm_INCLUDE_FLAGS) -o $(pass_dir)/obj/$(notdir $@)
	@$(CC) -shared -o $(pass_dir)/dll/$(basename $(notdir $<))$(DLL_EXT) $(pass_dir)/obj/$(basename $(notdir $<)).o $(libfirm_STATIC_PATH) -Wl,-rpath=$(libfirm_DYNAMIC_PATH) -lfirm
	@echo Build $(basename $(notdir $<))$(DLL_EXT)
	