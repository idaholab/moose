# This file contains common MOOSE application settings
# Note: MOOSE applications are assumed to reside in peer directories relative to MOOSE and optionally ELK.
#       This can be overridden by using environment variables (MOOSE_DIR and/or ELK_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.

##############################################################################
######################### Application Variables ##############################
##############################################################################
#
# source files
SRC_DIRS    := $(CURR_DIR)/src

srcfiles    := $(shell find $(SRC_DIRS) -name *.C)
csrcfiles   := $(shell find $(SRC_DIRS) -name *.c)
fsrcfiles   := $(shell find $(SRC_DIRS) -name *.f)
f90srcfiles := $(shell find $(SRC_DIRS) -name *.f90)

#
# object files
objects	    := $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
cobjects    := $(patsubst %.c, %.$(obj-suffix), $(csrcfiles))
fobjects    := $(patsubst %.f, %.$(obj-suffix), $(fsrcfiles))
f90objects  := $(patsubst %.f90, %.$(obj-suffix), $(f90srcfiles))

#
# header files

# Note: libMesh contains a Factory and Transient class which are "hidden"
#       by the order of includes here on case insensitive filesystems.
#       If we need to use either of these classes from libMesh directly
#       in Moose, we will have problems until one of them is renamed or
#       resolved in some other fashion
app_DIRS	+= $(shell find $(CURR_DIR) -type d | grep -v .svn)
moose_INCLUDE   := $(foreach i, $(app_DIRS), -I$(i)) $(ADDITIONAL_INCLUDES)
libmesh_INCLUDE := $(moose_INCLUDE) $(libmesh_INCLUDE)

###############################################################################
# Target:
#
target	:= ./$(APPLICATION_NAME)-$(METHOD)

###############################################################################
# Build Rules:
#
all:: $(target)

# add MOOSE dependency
ifdef moose_LIB
$(target): $(moose_LIB)
endif

# add possible ELK dependency
ifdef elk_LIB
$(target): $(elk_LIB)
endif

# Normal Executable
$(target): $(fobjects) $(f90objects) $(objects) $(cobjects) $(mesh_library) $(ADDITIONAL_DEPS)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) $(cobjects) $(fobjects) $(f90objects) -o $@ $(LIBS) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(ADDITIONAL_LIBS)

clean::
	@rm -fr $(APPLICATION_NAME)-* lib$(APPLICATION_NAME)-* $(exodiff)
	@find . -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" \
                -or -name "*.gcda" -or -name "*.gcno" -or -name "*.gcov" \
                -or -name "*.mod" | xargs rm
	@rm -fr *.mod

# Clean only the opt intermediate files
cleanopt::
	@rm -fr $(APPLICATION_NAME)-opt* lib$(APPLICATION_NAME)-opt*
	@find . -name "*opt.o" -or -name "*opt.d" | xargs rm

# Clean only the dbg intermediate files
cleandbg::
	@rm -fr $(APPLICATION_NAME)-dbg* lib$(APPLICATION_NAME)-dbg*
	@find . -name "*dbg.o" -or -name "*dbg.d" | xargs rm

# Clean only the prof intermediate files
cleanpro::
	@rm -fr $(APPLICATION_NAME)-pro* lib$(APPLICATION_NAME)-pro*
	@find . -name "*pro.o" -or -name "*pro.d" | xargs rm


cleanall::
	$(MAKE) clean

#
# Dependencies
#

# include the dependency list
-include src/*.d
-include src/*/*.d
-include src/*/*/*.d
-include src/*/*/*/*.d


