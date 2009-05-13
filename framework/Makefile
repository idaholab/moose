# $Id: Makefile 2501 2007-11-20 02:33:29Z benkirk $


# The location of the mesh library
meshdir := $(LIBMESH_DIR)

# include the library options determined by configure.  This will
# set the variables INCLUDE and LIBS that we will need to build and
# link with the library.
include $(meshdir)/Make.common

libmesh_INCLUDE += -Iinclude -Iinclude/materials

###############################################################################
# File management.  This is where the source, header, and object files are
# defined

#
# source files
srcfiles 	:= $(wildcard src/*.C) $(wildcard src/materials/*.C)

#
# object files
objects		:= $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
###############################################################################


.PHONY: clean doc

###############################################################################
# Target:
#
target 	   := ./libmoose-$(METHOD).a

all:: Makefile.export $(target)

# Production rules:  how to make the target - depends on library configuration
#	@echo "Linking "$@"..."
#	libtool -static -o $(target) $(objects)
#	$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)


#@echo "MOOSE_DIR := " $(PWD) > Makefile.export
Makefile.export: exported_stuff
	@echo "Creating Makefile.export"
	@cat exported_stuff >> Makefile.export

ifeq ($(findstring darwin,$(hostos)),darwin)
$(target): $(objects)
	@echo "Linking "$@
	@libtool -static -o $(target) $(objects)

else
$(target): $(objects)
	@echo "Linking "$@
	@$(AR) rv $(target) $(objects)
endif
# shared library
#
# (target): $(objects)
# 	@$(shell mkdir -p $(mesh_library_dir))
# 	@echo "Linking "$@
# 	@$(libmesh_CXX) $(libmesh_CXXSHAREDFLAG) -o $(mesh_library) $(objects) $(libmesh_LDFLAGS)
# 	@$(MAKE) -C contrib


doc:
	doxygen ./doc/Doxyfile


# Useful rules.
clean:
	@rm -f bison-*
	@find . -name "*~" -or -name "*.o" | xargs rm
	@rm -f .depend
	@rm -f Makefile.export

# include the dependency list
include .depend


#
# Dependencies
#
.depend:
	@$(perl) $(meshdir)/contrib/bin/make_dependencies.pl -I. $(foreach i, $(wildcard $(meshdir)/include/*), -I$(i)) -Iinclude -Iinclude/materials "-S\$$(obj-suffix)" $(srcfiles) > .depend
	@echo "Updated .depend"

###############################################################################
