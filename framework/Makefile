 # $Id: Makefile 2501 2007-11-20 02:33:29Z benkirk $ 

MOOSE_DIR ?= $(shell pwd)

# The location of the mesh library
meshdir := $(LIBMESH_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
include $(meshdir)/Make.common

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
target := $(MOOSE_DIR)/libmoose-$(METHOD)$(static_libext)

ifeq ($(enable-shared),yes)
  target := $(MOOSE_DIR)/libmoose-$(METHOD)$(shared_libext)
endif

all:: Makefile.export $(target)

#@echo "MOOSE_DIR := " $(PWD) > Makefile.export
Makefile.export: Makefile.export.in
	@echo "Creating Makefile.export"
	@cat Makefile.export.in >> Makefile.export

ifeq ($(enable-shared),yes)
# Build dynamic library
$(target): $(objects)
	@echo "Linking "$@
	@$(libmesh_CC) $(libmesh_CXXSHAREDFLAG) -o $@ $(objects) $(libmesh_LDFLAGS)
else
# Build static library
ifeq ($(findstring darwin,$(hostos)),darwin)
$(target): $(objects)
	@echo "Linking "$@
	@libtool -static -o $@ $(objects)
else
$(target): $(objects)
	@echo "Linking "$@
	@$(AR) rv $@ $(objects)
endif
endif


doc:
	doxygen ./doc/Doxyfile


# Useful rules.
clean:
	@rm -f libmoose-*
	@find . -name "*~" -or -name "*.o" | xargs rm
	@rm -f .depend
	@rm -f Makefile.export

# include the dependency list
include .depend


#
# Dependencies
#
.depend:
	@$(perl) $(meshdir)/contrib/bin/make_dependencies.pl -I. $(foreach i, $(wildcard $(meshdir)/include/*), -I$(i)) -Iinclude -Iinclude/stabilizers -Iinclude/executioners -Iinclude/base -Iinclude/ics -Iinclude/kernels -Iinclude/bcs -Iinclude/auxkernels -Iinclude/utils -Iinclude/materials -Iinclude/parser "-S\$$(obj-suffix)" $(srcfiles) > .depend

###############################################################################
