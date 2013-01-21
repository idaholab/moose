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

# plugin files
plugfiles   := $(shell find $(CURR_DIR)/plugins -name *.C 2>/dev/null)
cplugfiles  := $(shell find $(CURR_DIR)/plugins -name *.c 2>/dev/null)
fplugfiles  := $(shell find $(CURR_DIR)/plugins -name *.f 2>/dev/null)
f90plugfiles:= $(shell find $(CURR_DIR)/plugins -name *.f90 2>/dev/null)

# plugins
plugins	    := $(patsubst %.C, %-$(METHOD).plugin, $(plugfiles))
plugins	    += $(patsubst %.c, %-$(METHOD).plugin, $(cplugfiles))
plugins	    += $(patsubst %.f, %-$(METHOD).plugin, $(fplugfiles))
plugins	    += $(patsubst %.f90, %-$(METHOD).plugin, $(f90plugfiles))

#
# header files

# Note: libMesh contains a Factory and Transient class which are "hidden"
#       by the order of includes here on case insensitive filesystems.
#       If we need to use either of these classes from libMesh directly
#       in Moose, we will have problems until one of them is renamed or
#       resolved in some other fashion
app_DIRS	+= $(shell find $(CURR_DIR)/include -type d | grep -v "\.svn")
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
$(target): $(fobjects) $(f90objects) $(objects) $(cobjects) $(mesh_library) $(ADDITIONAL_DEPS) $(plugins)
	@echo "MOOSE Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) $(objects) $(cobjects) $(fobjects) $(f90objects) -o $@ $(LIBS) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) $(ADDITIONAL_LIBS)


delete_list := $(APPLICATION_NAME)-* lib$(APPLICATION_NAME)-*

# Clean only the opt intermediate files
cleanopt::
	@rm -fr $(APPLICATION_NAME)-opt* lib$(APPLICATION_NAME)-opt*
	@find . \( -name "*opt.o" -or -name "*opt.d" \) -exec rm '{}' \;

# Clean only the dbg intermediate files
cleandbg::
	@rm -fr $(APPLICATION_NAME)-dbg* lib$(APPLICATION_NAME)-dbg*
	@find . \( -name "*dbg.o" -or -name "*dbg.d" \) -exec rm '{}' \;

# Clean only the prof intermediate files
cleanpro::
	@rm -fr $(APPLICATION_NAME)-pro* lib$(APPLICATION_NAME)-pro*
	@find . \( -name "*pro.o" -or -name "*pro.d" \) -exec rm '{}' \;


cleanall::
	$(MAKE) clean

