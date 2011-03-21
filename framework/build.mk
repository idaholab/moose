# This file contains common MOOSE application settings
# Note: MOOSE applications are assumed to reside in peer directories relative to MOOSE and optionally ELK.
#       This can be overridden by using environment variables (MOOSE_DIR and/or ELK_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
include $(LIBMESH_DIR)/Make.common

libmesh_CXXFLAGS     += -MD
libmesh_CFLAGS       += -MD

# Fortran baggage
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
	libmesh_FFLAGS += -fdefault-real-8
endif

# g95
ifneq (,$(findstring g95,$(mpif77_command)))
	libmesh_FFLAGS += -r8
endif

# compile with gcov support if using the gcc compiler suite
ifeq ($(coverage),true)
	ifneq (,$(findstring gcc,$(GXX-VERSION)))
		libmesh_CXXFLAGS += --coverage -fbranch-probabilities 
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

##############################################################################
######################### Application Variables ##############################
##############################################################################
#
# source files
srcfiles    := $(shell find $(CURR_DIR) -name *.C)
csrcfiles    := $(shell find $(CURR_DIR) -name *.c)

ifeq ($(MAKE_LIBRARY),yes)
# THIS LINE SHOULD BE MADE MORE GENERIC
srcfiles    := $(filter-out ./src/main.C, $(srcfiles))
endif

fsrcfiles   := $(shell find $(CURR_DIR) -name *.f)
f90srcfiles := $(shell find $(CURR_DIR) -name *.f90)

#
# object files
objects	    := $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
cobjects	:= $(patsubst %.c, %.$(obj-suffix), $(csrcfiles))
fobjects    := $(patsubst %.f, %.$(obj-suffix), $(fsrcfiles))
f90objects  := $(patsubst %.f90, %.$(obj-suffix), $(f90srcfiles))

#
# header files
app_DIRS	+= $(shell find $(CURR_DIR) -type d | grep -v .svn)
libmesh_INCLUDE += -Iinclude $(foreach i, $(app_DIRS), -I$(i)) $(ADDITIONAL_INCLUDES)

.PHONY: clean doc 

###############################################################################
# Target:
#
ifeq ($(MAKE_LIBRARY),yes)
APPLICATION_NAME := lib$(APPLICATION_NAME)
target := $(CURR_DIR)/$(APPLICATION_NAME)-$(METHOD)$(static_libext)

ifeq ($(enable-shared),yes)
	target := $(CURR_DIR)/$(APPLICATION_NAME)-$(METHOD)$(shared_libext)
endif
else
target	:= ./$(APPLICATION_NAME)-$(METHOD)
endif

###############################################################################
# Build Rules:
#
all:: $(target)

ifeq ($(MAKE_LIBRARY),yes)
ifeq ($(enable-shared),yes)
# Build dynamic library
$(target): $(fobjects) $(f90objects) $(objects) $(cobjects)
	@echo "Linking "$@"..."
	@$(libmesh_CC) $(libmesh_CXXSHAREDFLAG) -o $@ $(fobjects) $(f90objects) $(objects) $(cobjects) $(libmesh_LDFLAGS)
else
# Build static library
ifeq ($(findstring darwin,$(hostos)),darwin)
$(target): $(fobjects) $(f90objects) $(objects) $(cobjects)
	@echo "Linking "$@"..."
	@libtool -static -o $@ $(fobjects) $(f90objects) $(objects) $(cobjects) 
else
$(target): $(fobjects) $(f90objects) $(objects) $(cobjects)
	@echo "Linking "$@"..."
	@$(AR) rv $@ $(fobjects) $(f90objects) $(objects) $(cobjects)
endif
endif
else

# Normal Executable
$(target): $(fobjects) $(f90objects) $(objects) $(cobjects) $(mesh_library) $(ADDITIONAL_DEPS)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) $(cobjects) $(fobjects) $(f90objects) -o $@ $(LIBS) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(ADDITIONAL_INCLUDES) $(ADDITIONAL_LIBS) 

endif

libonly:
	@$(MAKE) MAKE_LIBRARY=yes

lib: 
	@$(MAKE) MAKE_LIBRARY=yes

doc:
	doxygen doc/Doxyfile

clean::
	@rm -f $(APPLICATION_NAME)-* lib$(APPLICATION_NAME)-*
	@find . -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" \
                -or -name "*.gcda" -or -name "*.gcno" -or -name "*.gcov" | xargs rm

# Clean only the opt intermediate files
cleanopt::
	@rm -f $(APPLICATION_NAME)-opt* lib$(APPLICATION_NAME)-opt*
	@find . -name "*opt.o" -or -name "*opt.d" | xargs rm

# Clean only the dbg intermediate files
cleandbg::
	@rm -f $(APPLICATION_NAME)-dbg* lib$(APPLICATION_NAME)-dbg*
	@find . -name "*dbg.o" -or -name "*dbg.d" | xargs rm

# Clean only the prof intermediate files
cleanpro::
	@rm -f $(APPLICATION_NAME)-pro* lib$(APPLICATION_NAME)-pro*
	@find . -name "*pro.o" -or -name "*pro.d" | xargs rm


cleanall::
	$(MAKE) clean

syntax:
	python scripts/generate_input_syntax.py
#
# Dependencies
#

# include the dependency list
-include src/*.d
-include src/*/*.d
-include src/*/*/*.d

