 #  $Id: Makefile 2501 2007-11-20 02:33:29Z benkirk $

#If MOOSE_DIR isn't found in the environment it's assumed
MOOSE_DIR ?= $(shell pwd)/../moose
CPPUNIT_DIR ?= $HOME/root/usr/local/packages/cppunit

ELK_DIR ?= $(shell pwd)/../elk

# The location of the mesh library
meshdir := $(LIBMESH_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
#include $(meshdir)/Make.common

-include $(MOOSE_DIR)/Makefile.export
ifeq ($(MAKECMDGOALS), elkunit-$(METHOD))
	-include $(ELK_DIR)/Makefile.export
endif

# pass the parameter coverage=true if you want to support gcov
ifeq ($(coverage),true)
	ifneq (,$(findstring gcc,$(GXX-VERSION)))
		libmesh_CXXFLAGS += --coverage
		libmesh_LDFLAGS += --coverage
	endif
endif

mpif77_command 	:= $(libmesh_F77)

###############################################################################
# File management.  This is where the source, header, and object files are
# defined

#
# source files
srcfiles 	:= $(wildcard src/*.C) $(wildcard src/*/*.C)

#
# object files
objects		:= $(patsubst %.C, %.$(MAKECMDGOALS)_$(obj-suffix), $(srcfiles))

# include dirs
libmesh_INCLUDE += -Iinclude -I$(CPPUNIT_DIR)/include

# lib dirs
CPPUNIT_LIB	:= -L$(CPPUNIT_DIR)/lib -lcppunit 
libmesh_LDFLAGS	+= $(libmesh_RPATHFLAG)$(CPPUNIT_LIB)
###############################################################################


.PHONY: clean doc mooseunit elkunit

###############################################################################
# Target:
#
all::	mooseunit


mooseunit: 
	@$(MAKE) mooseunit-$(METHOD)

elkunit: 
	@$(MAKE) elkunit-$(METHOD) libmesh_CXXFLAGS="$(libmesh_CXXFLAGS) -DELK_TEST"

# Production rules:  how to make the target - depends on library configuration
mooseunit-$(METHOD): $(objects) $(moose_LIB) $(mesh_library)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)

elkunit-$(METHOD):  $(objects) $(moose_LIB) $(mesh_library)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)


###############################################################################
# Special Rules for augmented obj_suffix from libmesh
#
%.$(MAKECMDGOALS)_$(obj-suffix) : %.C
	@echo "Compiling C++ (in "$(mode)" mode) "$<"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) -c $< -o $@


#doc:
#	doxygen ./doc/Doxyfile

# Useful rules.
clean:
	@rm -f mooseunit-* elkunit-*
	@find . -name "*~" -or -name "*.o" | xargs rm
	@rm -f .depend

# include the dependency list
-include .depend

#
# Dependencies
#
.depend:
	@$(perl) $(meshdir)/contrib/bin/make_dependencies.pl -I. $(libmesh_INCLUDE) "-S\$$(obj-suffix)" $(srcfiles) > .depend
	@echo "Updated .depend"

###############################################################################
