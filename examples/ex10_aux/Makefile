#  $Id: Makefile 2501 2007-11-20 02:33:29Z benkirk $

#If MOOSE_DIR isn't found in the environment it's assumed
MOOSE_DIR ?= ../../moose

#If PF_DIR isn't found in the environment it's assumed
PF_DIR ?= ../pf

HYPRE_LINK_FLAGS=

# The location of the mesh library
meshdir := $(LIBMESH_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
#include $(meshdir)/Make.common

-include $(MOOSE_DIR)/Makefile.export

mpif77_command := $(shell $(libmesh_F77) -show)

ifneq (,$(findstring ifort,$(mpif77_command)))
	libmesh_FFLAGS += -r8
else
	libmesh_FFLAGS += -fdefault-real-8
endif

###############################################################################
# File management.  This is where the source, header, and object files are
# defined

#
# source files
srcfiles 	:= $(wildcard src/*.C) $(wildcard src/*/*.C)
fsrcfiles       := $(wildcard src/matpro/*.f)

#
# object files
objects		:= $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
fobjects        := $(patsubst %.f, %.$(obj-suffix), $(fsrcfiles))

#
# phasefield objects and build rule
ex7PFobjects 	:= $(patsubst %.C, %-pf.$(obj-suffix), $(srcfiles))
pfobjects 	:= $(PF_DIR)/src/pf.o 
%-pf.$(obj-suffix) : %.C
	@echo "Compiling C++ (in "$(mode)" mode) "$<"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) -c $< -o $@


# include dirs
libmesh_INCLUDE += -Iinclude -Iinclude/base -Iinclude/ics -Iinclude/kernels -Iinclude/bcs -Iinclude/auxkernels -Iinclude/utils -Iinclude/materials -Iinclude/parser
###############################################################################


.PHONY: clean doc

###############################################################################
# Target:
#
target 	   := ./ex10-$(METHOD)


all:: $(target)

# Production rules:  how to make the target - depends on library configuration
$(target): $(fobjects) $(objects) $(moose_LIB) $(mesh_library)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) $(fobjects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)

with-pf:
#	@touch src/Ex7.C src/materials/Meso.C src/materials/MesoData.C
	@$(MAKE) with-pf_ libmesh_CXXFLAGS="$(libmesh_CXXFLAGS) -DWITH_PF" libmesh_INCLUDE="$(libmesh_INCLUDE) -I$(PF_DIR)/include"

with-pf_: $(ex7PFobjects) $(fobjects) $(moose_LIB) $(mesh_library)
	@echo "Linking "$(target)-pf"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(ex7PFobjects) $(pfobjects) $(fobjects) -o $(target)-pf $(libmesh_LIBS) $(libmesh_LDFLAGS) 	$(HYPRE_LINK_FLAGS)

doc:
	doxygen ./doc/Doxyfile

# Useful rules.
clean:
	@rm -f ex10-*
	@find . -name "*~" -or -name "*.o" | xargs rm
	@rm -f .depend

# include the dependency list
-include .depend

print:
	@echo $(mpif77_command)
	@echo $(libmesh_F77)
	@echo $(libmesh_FFLAGS)

#
# Dependencies
#
.depend:
	@$(perl) $(meshdir)/contrib/bin/make_dependencies.pl -I. $(libmesh_INCLUDE) "-S\$$(obj-suffix)" $(srcfiles) > .depend
	@echo "Updated .depend"

###############################################################################
