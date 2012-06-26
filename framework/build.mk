# This file contains common MOOSE application settings
# Note: MOOSE applications are assumed to reside in peer directories relative to MOOSE and optionally ELK.
#       This can be overridden by using environment variables (MOOSE_DIR and/or ELK_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.

include $(LIBMESH_DIR)/Make.common

# Number of JOBS to run in parallel used in run_tests
JOBS ?= 1

all::

#
# C++ rules
#

%.$(obj-suffix) : %.C
	@echo "Compiling C++ (in "$(mode)" mode) "$<"..."
	@$(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) -MMD -MF $@.d $(libmesh_INCLUDE) -c $< -o $@

#
# C rules
#

%.$(obj-suffix) : %.c
	@echo "Compiling C (in "$(mode)" mode) "$<"..."
	@$(libmesh_CC) $(libmesh_CPPFLAGS) $(libmesh_CFLAGS) -MMD -MF $@.d $(libmesh_INCLUDE) -c $< -o $@

#
# Fortran77 rules
#

%.$(obj-suffix) : %.f
	@echo "Compiling Fortran (in "$(mode)" mode) "$<"..."
	@$(libmesh_F77) $(libmesh_FFLAGS) $(libmesh_INCLUDE) -c $< -o $@

# preprocessed Fortran
PreProcessed_FFLAGS := $(libmesh_FFLAGS)
%.$(obj-suffix) : %.F
	@echo "Compiling Fortran (in "$(mode)" mode) "$<"..."
	@$(libmesh_F90) $(PreProcessed_FFLAGS) $(libmesh_INCLUDE) -c $< $(module_dir_flag) -o $@

#
# Fortran90 rules
#

mpif90_command := $(libmesh_F90)

# If $(libmesh_f90) is an mpiXXX compiler script, use -show
# to determine the base compiler
ifneq (,$(findstring mpi,$(mpif90_command)))
	mpif90_command = $(shell $(libmesh_F90) -show)
endif

# module_dir_flag is a flag that, if defined, instructs the compiler
# to put any .mod files in the directory where the obect files are put.

#ifort
ifneq (,$(findstring ifort,$(mpif90_command)))
	module_dir_flag = -module ${@D}
endif

#gfortran
ifneq (,$(findstring gfortran,$(mpif90_command)))
	module_dir_flag = -J${@D}
endif

%.$(obj-suffix) : %.f90
	@echo "Compiling Fortran90 (in "$(mode)" mode) "$<"..."
	@$(libmesh_F90) $(libmesh_FFLAGS) $(libmesh_INCLUDE) -c $< $(module_dir_flag) -o $@


# treat these warnings as errors (This doesn't seem to be necessary for Intel)
ifneq (,$(findstring gcc,$(GXX-VERSION)))
  libmesh_CXXFLAGS     += -Werror=return-type -Werror=reorder
endif

#
# Fortran baggage
#
mpif77_command := $(libmesh_F77)

# If $(libmesh_f77) is an mpiXXX compiler script, use -show
# to determine the base compiler
ifneq (,$(findstring mpi,$(mpif77_command)))
	mpif77_command := $(shell $(libmesh_F77) -show)
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
	ifneq (,$(findstring gcc,$(GXX-VERSION)))
		libmesh_CXXFLAGS += --coverage -fprofile-arcs -ftest-coverage
		libmesh_LDFLAGS += --coverage
	endif
endif

# link with gcov support, but do now generate data for this build
# if you wanted code coverage data for moose, but you wanted to run
# the tests in moose_test you would make moose with coverage=true
# and moose_test with just linkcoverage=true
ifeq ($(linkcoverage),true)
	ifneq (,$(findstring gcc,$(GXX-VERSION)))
		libmesh_LDFLAGS += --coverage
	endif
endif

# graceful error exiting
ifeq ($(graceful),true)
	libmesh_CXXFLAGS += -DGRACEFUL_ERROR
endif

#
# Variables
#

# library type sets different options
ifeq ($(enable-shared),yes)
  libext := $(shared_libext)
	LIBS += -Wl,-rpath,$(MOOSE_DIR)
else
  libext := $(static_libext)
endif

CURRENT_APP ?= $(shell basename `pwd`)

# Build appliations up the tree
up: all
	@echo ====== Building the following applications: $(CURRENT_APP) $(DEP_APPS) ======
	@for app in $(CURRENT_APP) $(DEP_APPS); \
	do \
		echo ====== Making in $${app} ====== ; \
		$(MAKE) -C $(ROOT_DIR)/$$app || break; \
	done


test_up: all up
	@echo ====== Testing the following applications: $(CURRENT_APP) $(DEP_APPS) ======
	@for app in $(CURRENT_APP) $(DEP_APPS); \
	do \
		echo ====== Testing in $${app} ====== ; \
		(cd $(ROOT_DIR)/$$app && ./run_tests -q -j $(JOBS)) ; \
	done

clean_up:
	@echo ====== Cleaning the following applications: $(CURRENT_APP) $(DEP_APPS) ======
	@for app in $(CURRENT_APP) $(DEP_APPS); \
	do \
		echo ====== Cleaning $${app} ====== ; \
		$(MAKE) -C $(ROOT_DIR)/$$app clean; \
	done

#
# Maintenance
#
.PHONY: cleanall clean doc

#
# Misc
#
syntax:
	python scripts/generate_input_syntax.py

doc:
	doxygen doc/doxygen/Doxyfile
