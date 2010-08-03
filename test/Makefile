 # $Id: Makefile 2501 2007-11-20 02:33:29Z benkirk $ 

MOOSE_DIR ?= ../moose

# The location of the mesh library
meshdir := $(LIBMESH_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
-include $(meshdir)/Make.common

-include $(MOOSE_DIR)/Makefile.export

libmesh_INCLUDE += -Iinclude -Iinclude/base -Iinclude/stabilizers -Iinclude/executioners -Iinclude/ics -Iinclude/kernels -Iinclude/bcs -Iinclude/auxkernels -Iinclude/utils -Iinclude/utils/fparser -Iinclude/materials -Iinclude/parser

###############################################################################
# File management.  This is where the source, header, and object files are
# defined

#
# source files
srcfiles 	:= $(wildcard src/*.C) $(wildcard src/*/*.C) $(wildcard src/*/*/*.C)

#
# object files
objects		:= $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
###############################################################################


.PHONY: clean doc .depend

###############################################################################
# Target:
#
target := ./moose_test-$(METHOD)

all:: $(target)

$(target): $(objects) $(moose_LIB) $(mesh_library)
	@echo "Linking "$@
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)
doc:
	doxygen ./doc/Doxyfile

# Useful rules.
clean:
	@rm -f moose_test-*
	@find . -name "*~" -or -name "*.o" | xargs rm
	@rm -f .depend
	@rm -f Makefile.export

# include the dependency list
-include .depend

#
# Dependencies
#
.depend:
	@$(perl) $(meshdir)/contrib/bin/make_dependencies.pl -I. $(foreach i, $(wildcard $(meshdir)/include/*), -I$(i)) -Iinclude -Iinclude/stabilizers -Iinclude/executioners -Iinclude/base -Iinclude/ics -Iinclude/kernels -Iinclude/bcs -Iinclude/auxkernels -Iinclude/utils -Iinclude/materials -Iinclude/parser "-S\$$(obj-suffix)" $(srcfiles) > .depend

###############################################################################
