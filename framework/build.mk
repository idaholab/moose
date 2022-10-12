# This file contains common MOOSE application settings
# Note: MOOSE applications are assumed to reside in peer directories relative to MOOSE.
#       This can be overridden by using environment variables (MOOSE_DIR and/or FRAMEWORK_DIR)

# Set LIBMESH_DIR if it is not already set in the environment (try our best to guess!)
LIBMESH_DIR       ?= $(MOOSE_DIR)/libmesh/installed

# Default number of parallel jobs to use for run_tests
MOOSE_JOBS        ?= 8

# Include variables defined by MOOSE configure if it's been run
-include $(MOOSE_DIR)/conf_vars.mk

# If the user has no environment variable
# called METHOD, they get optimized mode.
ifeq (x$(METHOD),x)
  METHOD := opt
endif

# libmesh-config is in different places depending on whether you are using
# "installed" or "uninstalled" libmesh.
libmesh_config := $(LIBMESH_DIR)/bin/libmesh-config           # installed version
ifeq ($(wildcard $(libmesh_config)),)
  libmesh_config := $(LIBMESH_DIR)/contrib/bin/libmesh-config # uninstalled version
endif

# Instead of using Make.common, use libmesh-config to get any libmesh
# make variables we might need.  Be sure to pass METHOD along to libmesh-config
# so that it can use the right one!
libmesh_CXX      ?= $(shell METHOD=$(METHOD) $(libmesh_config) --cxx)
libmesh_CC       ?= $(shell METHOD=$(METHOD) $(libmesh_config) --cc)
libmesh_F77      ?= $(shell METHOD=$(METHOD) $(libmesh_config) --fc)
libmesh_F90      ?= $(shell METHOD=$(METHOD) $(libmesh_config) --fc)
libmesh_INCLUDE  := $(shell METHOD=$(METHOD) $(libmesh_config) --include)
libmesh_CPPFLAGS := $(shell METHOD=$(METHOD) $(libmesh_config) --cppflags)
libmesh_CXXFLAGS := $(shell METHOD=$(METHOD) $(libmesh_config) --cxxflags)
libmesh_CFLAGS   := $(shell METHOD=$(METHOD) $(libmesh_config) --cflags)
libmesh_FFLAGS   := $(shell METHOD=$(METHOD) $(libmesh_config) --fflags)
libmesh_LIBS     := $(shell METHOD=$(METHOD) $(libmesh_config) --libs)
libmesh_HOST     := $(shell METHOD=$(METHOD) $(libmesh_config) --host)
libmesh_LDFLAGS  := $(shell METHOD=$(METHOD) $(libmesh_config) --ldflags)

# You can completely disable timing by setting MOOSE_NO_PERF_GRAPH in your environment
ifneq (x$(MOOSE_NO_PERF_GRAPH), x)
  libmesh_CXXFLAGS += -DMOOSE_NO_PERF_GRAPH
endif

ifneq ($(GPERF_DIR), )
ifeq ($(METHOD),$(filter $(METHOD), dbg devel))
    $(error It does not make sense to profile with the $(METHOD) method due to assertions. Please unset GPERF_DIR)
else
    libmesh_CXXFLAGS += -DHAVE_GPERFTOOLS -I$(GPERF_DIR)/include
    libmesh_LDFLAGS := -L$(GPERF_DIR)/lib -Wl,-rpath,$(GPERF_DIR)/lib -ltcmalloc_and_profiler $(libmesh_LDFLAGS)
endif
endif

# Google Test relies on static construction of objects in test
# compilation units to register those tests, but with some Linux
# distributions (Ubuntu 21.04 for me; others in
# https://github.com/idaholab/moose/issues/16092 ) shared libraries
# don't get loaded by default (and thus don't call constructors of
# static objects by default) unless the shared library satisfies a
# missing symbol, or unless we force it to load with a special linker
# flag.  The flag may require GCC or GNU ld, and if we don't support
# it it's not safe to use it, so test first.
NO_AS_NEEDED_FLAG = -Wl,--no-as-needed
HAVE_NO_AS_NEEDED := $(shell echo 'int main(void){return 0;}' > as_needed_test.C; if $(libmesh_CXX) $(NO_AS_NEEDED_FLAG) as_needed_test.C -o as_needed_test.x 2>/dev/null; then echo yes; else echo no; fi; rm -f as_needed_test.C as_needed_test.x)

ifeq ($(HAVE_NO_AS_NEEDED),yes)
  libmesh_LDFLAGS += $(NO_AS_NEEDED_FLAG)
endif

# Make.common used to provide an obj-suffix which was related to the
# machine in question (from config.guess, i.e. @host@ in
# contrib/utils/Make.common.in) and the $(METHOD).
obj-suffix := $(libmesh_HOST).$(METHOD).lo
no-method-obj-suffix := $(libmesh_HOST).lo

# The libtool script used by libmesh is in different places depending on
# whether you are using "installed" or "uninstalled" libmesh.
libmesh_LIBTOOL := $(LIBMESH_DIR)/contrib/bin/libtool # installed version
ifeq ($(wildcard $(libmesh_LIBTOOL)),)
  libmesh_LIBTOOL := $(LIBMESH_DIR)/libtool           # uninstalled version
endif
libmesh_shared  := $(shell $(libmesh_LIBTOOL) --config | grep build_libtool_libs | cut -d'=' -f2)
libmesh_static  := $(shell $(libmesh_LIBTOOL) --config | grep build_old_libs | cut -d'=' -f2)

#
# libPNG Definition
#
png_LIB         :=
# There is a linking problem where we have undefined symbols in the png library on Linux
# systems that we have not resolved. We won't attempt to use libpng with static
ifneq ($(libmesh_static),yes)
  # See conf_vars.mk
  png_LIB          := $(libPNG_LIBS)
  libmesh_CXXFLAGS += $(libPNG_INCLUDE)
endif

# If $(libmesh_CXX) is an mpiXXX compiler script, use -show
# to determine the base compiler
cxx_compiler := $(libmesh_CXX)
ifneq (,$(findstring mpi,$(cxx_compiler)))
	cxx_compiler := $(shell $(libmesh_CXX) -show)
endif

all:

# Add all header symlinks as dependencies to this target
header_symlinks:

unity_files:


#
# C++ rules
#
pcre%.$(obj-suffix) : pcre%.cc
	@echo "Compiling C++ (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(ADDITIONAL_CPPFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -w -DHAVE_CONFIG_H -MMD -MP -MF $@.d -MT $@ -c $< -o $@

gtest%.$(no-method-obj-suffix) : gtest%.cc
	@echo "Compiling C++ "$<"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(ADDITIONAL_CPPFLAGS) $(CXXFLAGS) -w -MMD -MP -MF $@.d -MT $@ -c $< -o $@

%.$(obj-suffix) : %.cc
	@echo "Compiling C++ (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(ADDITIONAL_CPPFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -DHAVE_CONFIG_H -MMD -MP -MF $@.d -MT $@ -c $< -o $@

define CXX_RULE_TEMPLATE
%$(1).$(obj-suffix) : %.C $(ADDITIONAL_SRC_DEPS)
ifeq ($(1),)
	@echo "Compiling C++ (in "$$(METHOD)" mode) "$$<"..."
else
	@echo "Compiling C++ with suffix (in "$$(METHOD)" mode) "$$<"..."
endif
	@$$(libmesh_LIBTOOL) --tag=CXX $$(LIBTOOLFLAGS) --mode=compile --quiet \
	  $$(libmesh_CXX) $$(libmesh_CPPFLAGS) $$(CXXFLAGS) $$(libmesh_CXXFLAGS) $$(ADDITIONAL_CPPFLAGS) $$(app_INCLUDES) $$(libmesh_INCLUDE) -MMD -MP -MF $$@.d -MT $$@ -c $$< -o $$@
endef
# Instantiate Rules
$(eval $(call CXX_RULE_TEMPLATE,))

%.$(obj-suffix) : %.cpp
	@echo "Compiling C++ (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
	  $(libmesh_CXX) $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(ADDITIONAL_CPPFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -MMD -MP -MF $@.d -MT $@ -c $< -o $@

#
# Static Analysis
#

%.plist.$(obj-suffix) : %.C
	@echo "Clang Static Analysis (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) --analyze $< -o $@

%.plist.$(obj-suffix) : %.cc
	@echo "Clang Static Analysis (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) --analyze $< -o $@

#
# C rules
#

pcre%.$(obj-suffix) : pcre%.c
	@echo "Compiling C (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=CC $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CC) $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS) $(libmesh_CFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -w -DHAVE_CONFIG_H -MMD -MP -MF $@.d -MT $@ -c $< -o $@

%.$(obj-suffix) : %.c
	@echo "Compiling C (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=CC $(LIBTOOLFLAGS) --mode=compile --quiet \
	  $(libmesh_CC) $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS) $(libmesh_CFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -MMD -MP -MF $@.d -MT $@ -c $< -o $@



#
# Fortran77 rules
#

%.$(obj-suffix) : %.f
	@echo "Compiling Fortan (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=F77 $(LIBTOOLFLAGS) --mode=compile --quiet \
	  $(libmesh_F77) $(libmesh_FFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -c $< -o $@


#
# Fortran77 (with C preprocessor) rules
#

PreProcessed_FFLAGS := $(libmesh_FFLAGS)
%.$(obj-suffix) : %.F
	@echo "Compiling Fortran (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=F77 $(LIBTOOLFLAGS) --mode=compile --quiet \
	  $(libmesh_F90) $(PreProcessed_FFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -c $< $(module_dir_flag) -o $@

#
# Fortran90 rules
#

mpif90_command := $(libmesh_F90)

# If $(libmesh_f90) is an mpiXXX compiler script, use -show
# to determine the base compiler
ifneq (,$(findstring mpi,$(mpif90_command)))
	mpif90_command := $(shell $(libmesh_F90) -show)
endif

# module_dir_flag is a flag that, if defined, instructs the compiler
# to put any .mod files in the directory where the obect files are put.

#ifort
ifneq (,$(findstring ifort,$(mpif90_command)))
	module_dir_flag = -module ${@D}
endif

#gfortran
ifneq (,$(findstring gfortran,$(mpif90_command)))
	module_dir_flag = -J ${@D}
endif

%.$(obj-suffix) : %.f90
	@echo "Compiling Fortran90 (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_LIBTOOL) --tag=FC $(LIBTOOLFLAGS) --mode=compile --quiet \
	  $(libmesh_F90) $(libmesh_FFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -c $< $(module_dir_flag) -o $@

# Add method to list of defines passed to the compiler
libmesh_CXXFLAGS += -DMETHOD=$(METHOD)

# treat these warnings as errors (This doesn't seem to be necessary for Intel)
ifneq (,$(findstring g++,$(cxx_compiler)))
  libmesh_CXXFLAGS += -Werror=return-type -Werror=reorder

	# Disable the long string warning from GCC
	# warning: string length ‘524’ is greater than the length ‘509’ ISO C90 compilers are required to support [-Woverlength-strings]
	libmesh_CXXFLAGS += -Woverlength-strings
endif

#
# Fortran baggage
#
mpif77_command := $(libmesh_F77)

# If $(libmesh_f77) is an mpiXXX compiler script, use -show
# to determine the base compiler
ifneq (,$(findstring mpi,$(mpif77_command)))
	mpif77_command := $(shell $(libmesh_F77) -show | cut -f1 -d' ')
endif

# Set certain flags based on compiler

# ifort
ifneq (,$(findstring ifort,$(mpif77_command)))
	libmesh_FFLAGS += -r8
endif

# gfortran
ifneq (,$(findstring gfortran,$(mpif77_command)))
	libmesh_FFLAGS += -fdefault-real-8 -fdefault-double-8
endif

# g95
ifneq (,$(findstring g95,$(mpif77_command)))
	libmesh_FFLAGS += -r8
endif

# compile with gcov support if using the gcc compiler suite
ifeq ($(coverage),true)
	libmesh_CXXFLAGS += -DCOVERAGE_ENABLED -fprofile-arcs -ftest-coverage
	ifeq (,$(findstring clang++,$(cxx_compiler)))
		libmesh_LDFLAGS += -lgcov
		libmesh_LIBS += -lgcov
	endif
endif

# link with gcov support, but do now generate data for this build
# if you wanted code coverage data for moose, but you wanted to run
# the tests in moose_test you would make moose with coverage=true
# and moose_test with just linkcoverage=true
ifeq ($(linkcoverage),true)
	libmesh_LDFLAGS += -lgcov
endif

# graceful error exiting
ifeq ($(graceful),true)
	libmesh_CXXFLAGS += -DGRACEFUL_ERROR
endif

#
# Variables
#

CURRENT_APP ?= $(shell basename `pwd`)

ifeq ($(CURRENT_APP),moose)
  CONTAINING_DIR_FULLPATH := $(shell dirname `pwd`)
  CONTAINING_DIR := $(shell basename $(CONTAINING_DIR_FULLPATH))
  ifeq ($(CONTAINING_DIR), devel)
    CURRENT_APP := "devel/moose"
  endif
endif

#
# Plugins
# TODO[JWP]: These plugins might also be able to use libtool...but it turned
# out to be more trouble than it was worth to get working.
#
PLUGIN_FLAGS := -shared -fPIC -Wl,-undefined,dynamic_lookup
%-$(METHOD).plugin : %.C
	# we add include/base so that MooseConfig.h can be found, which is absent from the symlink dirs
	@$(libmesh_CXX) $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(PLUGIN_FLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) -I $(FRAMEWORK_DIR)/include/base $< -o $@
%-$(METHOD).plugin : %.c
	@echo "Compiling C Plugin (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_CC) $(libmesh_CPPFLAGS) $(ADDITIONAL_CPPFLAGS) $(libmesh_CFLAGS) $(PLUGIN_FLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) $< -o $@
%-$(METHOD).plugin : %.f
	@echo "Compiling Fortan Plugin (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_F77) $(libmesh_FFLAGS) $(PLUGIN_FLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) $< -o $@
%-$(METHOD).plugin : %.f90
	@echo "Compiling Fortan Plugin (in "$(METHOD)" mode) "$<"..."
	@$(libmesh_F90) $(libmesh_FFLAGS) $(PLUGIN_FLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) $< -o $@

# Define the "test" target, we'll use a variable name so that we can override it without warnings if needed
TEST ?= test
$(TEST): all
	@echo ======================================================
	@echo Testing $(CURRENT_APP)
	@echo ======================================================
	@(./run_tests -j $(MOOSE_JOBS))

libmesh_update:
	@echo ======================================================
	@echo Downloading and updating libMesh
	@echo ======================================================
	$(MOOSE_DIR)/scripts/update_and_rebuild_libmesh.sh

libmesh:
	@echo ======================================================
	@echo Updating libMesh
	@echo ======================================================
	$(MOOSE_DIR)/scripts/update_and_rebuild_libmesh.sh --fast

show_libmesh_configs:
	@echo "libmesh_CXX     :" $(libmesh_CXX)
	@echo "libmesh_CC      :" $(libmesh_CC)
	@echo "libmesh_F77     :" $(libmesh_F77)
	@echo "libmesh_F90     :" $(libmesh_F90)
	@echo "libmesh_INCLUDE :" $(libmesh_INCLUDE)
	@echo "libmesh_CPPFLAGS:" $(libmesh_CPPFLAGS)
	@echo "libmesh_CXXFLAGS:" $(libmesh_CXXFLAGS)
	@echo "libmesh_CFLAGS  :" $(libmesh_CFLAGS)
	@echo "libmesh_FFLAGS  :" $(libmesh_FFLAGS)
	@echo "libmesh_LIBS    :" $(libmesh_LIBS)
	@echo "libmesh_HOST    :" $(libmesh_HOST)
	@echo "libmesh_LDFLAGS :" $(libmesh_LDFLAGS)

#
# Maintenance
#
.PHONY: cleanall clean doc sa test

#
# Misc
#
syntax:
	$(shell python scripts/generate_input_syntax.py)

doc:
	$(shell doxygen doc/doxygen/Doxyfile)

depclean: cleandep
cleandep: cleandep
cleandeps: cleandep

cleandep:
	@for app in $(DEP_APPS); \
	do \
		@echo @python $(FRAMEWORK_DIR)/scripts/rm_outdated_deps.py $$app; \
	done
