
.PHONY = all clean

CC ?= gcc
FC ?= gfortran

SRCS = $(wildcard *.f90)
MODS =  $(wildcard *.mod)

GZ = $(patsubst %.mod,%.gz,$(MODS))

ifeq ($(OS),Windows_NT)
    uname_S := Windows
else
    uname_S := $(shell uname -s)
endif

ifneq (,$(findstring _NT,$(uname_S)))
   uname_S := Windows
endif

ifeq ($(uname_S), Windows)
   LIB_FLAGS = -shared
   LIB_SUFFIX = dll
   LIBS = $(patsubst %.f90,%.dll,$(SRCS))
else ifeq ($(uname_S),Linux)
   LIB_FLAGS = -fPIC -shared
   LIB_SUFFIX = so
   LIBS = $(patsubst %.f90,%.so,$(SRCS))
else ifeq ($(uname_S),Darwin)
   LIB_FLAGS = -dynamiclib
   LIB_SUFFIX = dylib
   LIBS = $(patsubst %.f90,%.dylib,$(SRCS))
else
$(error Unknown OS $(OS) $(uname_S))
endif


C_HELPERS = cdesc_test_helper.$(LIB_SUFFIX)
ARCH := $(shell uname -m)
ifndef FLANG_RT_DIR
FLANG_RT_DIR := $(shell ls -d /usr/lib/clang/*/lib/$(ARCH)-redhat-linux-gnu /usr/lib/llvm-*/lib 2>/dev/null | sort -V | tail -n 1)
endif

ifeq ($(CC),icx)
   C_HELPER_LIBS = -fortlib
   OPTIONS = -O2 --warn-all -fcheck=all
else ifneq (,$(findstring flang,$(notdir $(FC))))
   # flang does not accept many gfortran-specific hardening/debug switches.
   OPTIONS ?= -O2
   ifneq (,$(wildcard $(FLANG_RT_DIR)/libflang_rt.runtime.so))
      C_HELPER_LIBS ?= -L$(FLANG_RT_DIR) -lflang_rt.runtime
   else ifneq (,$(wildcard $(FLANG_RT_DIR)/libflang_rt.runtime.a))
      C_HELPER_LIBS ?= -L$(FLANG_RT_DIR) -lflang_rt.runtime
   else ifneq (,$(wildcard $(FLANG_RT_DIR)/libFortranRuntime.so))
      C_HELPER_LIBS ?= -L$(FLANG_RT_DIR) -lFortranRuntime
   else ifneq (,$(wildcard $(FLANG_RT_DIR)/libFortranRuntime.a))
      C_HELPER_LIBS ?= -L$(FLANG_RT_DIR) -lFortranRuntime
   else
      C_HELPER_LIBS ?= -lFortranRuntime
   endif
else
   C_HELPER_LIBS ?= -lgfortran
   OPTIONS ?= -ggdb -D_FORTIFY_SOURCE=2 -ffree-line-length-none  -ffree-form -fstack-clash-protection -fstack-protector-all -fstack-protector
endif

all: $(LIBS) $(C_HELPERS) $(GZ)

cdesc_test_helper.$(LIB_SUFFIX): cdesc_test_helper.c
	$(CC) $(LIB_FLAGS) -I.. -o $@ $< $(C_HELPER_LIBS)

%.$(LIB_SUFFIX): %.f90
	$(FC) $(OPTIONS) $(LIB_FLAGS) -cpp -o  $@ $<

%.gz: %.mod
	@cp $< $@
	@gunzip -f $@ -c > $@.extract
	@rm $@

%.o %.mod %.smod: %.f90
	$(FC) $(OPTIONS) $(LIB_FLAGS) -cpp -o  $@ $<

clean:
	-rm -f *.o *.mod *.gz *.pyc *.f90.*.* *.$(LIB_SUFFIX) *.original *.extract *.smod *.fpy
