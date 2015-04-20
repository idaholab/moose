# This file contains common MOOSE application settings
# Note: MOOSE applications are assumed to reside in peer directories relative to MOOSE and its modules.
#       This can be overridden by using the MOOSE_DIR environment variable

-include $(APPLICATION_DIR)/$(APPLICATION_NAME).mk

##############################################################################
######################### Application Variables ##############################
##############################################################################
#
# source files
SRC_DIRS    := $(APPLICATION_DIR)/src
PLUGIN_DIR  := $(APPLICATION_DIR)/plugins

srcfiles    := $(shell find $(SRC_DIRS) -name "*.C" -not -name main.C)
csrcfiles   := $(shell find $(SRC_DIRS) -name "*.c")
fsrcfiles   := $(shell find $(SRC_DIRS) -name "*.f")
f90srcfiles := $(shell find $(SRC_DIRS) -name "*.f90")

# object files
objects	    := $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
cobjects    := $(patsubst %.c, %.$(obj-suffix), $(csrcfiles))
fobjects    := $(patsubst %.f, %.$(obj-suffix), $(fsrcfiles))
f90objects  := $(patsubst %.f90, %.$(obj-suffix), $(f90srcfiles))
app_objects := $(objects) $(cobjects) $(fobjects) $(f90objects) $(ADDITIONAL_APP_OBJECTS)

# plugin files
plugfiles   := $(shell find $(PLUGIN_DIR) -name "*.C" 2>/dev/null)
cplugfiles  := $(shell find $(PLUGIN_DIR) -name "*.c" 2>/dev/null)
fplugfiles  := $(shell find $(PLUGIN_DIR) -name "*.f" 2>/dev/null)
f90plugfiles:= $(shell find $(PLUGIN_DIR) -name "*.f90" 2>/dev/null)

# plugins
plugins	    := $(patsubst %.C, %-$(METHOD).plugin, $(plugfiles))
plugins	    += $(patsubst %.c, %-$(METHOD).plugin, $(cplugfiles))
plugins	    += $(patsubst %.f, %-$(METHOD).plugin, $(fplugfiles))
plugins	    += $(patsubst %.f90, %-$(METHOD).plugin, $(f90plugfiles))

# main
main_src    := $(APPLICATION_DIR)/src/main.C    # Main must be located here!
main_object := $(patsubst %.C, %.$(obj-suffix), $(main_src))

# dependency files
app_deps    := $(patsubst %.C, %.$(obj-suffix).d, $(srcfiles)) \
               $(patsubst %.c, %.$(obj-suffix).d, $(csrcfiles)) \
               $(patsubst %.C, %.$(obj-suffix).d, $(main_src)) \
               $(ADDITIONAL_APP_DEPS)

depend_dirs := $(foreach i, $(DEPEND_MODULES), $(MOOSE_DIR)/modules/$(i)/include)
depend_dirs += $(APPLICATION_DIR)/include

# header files
include_dirs	:= $(shell find $(depend_dirs) -type d | grep -v "\.svn")
app_INCLUDE     := $(foreach i, $(include_dirs), -I$(i)) $(ADDITIONAL_INCLUDES)

# clang static analyzer files
app_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(srcfiles))

# library
app_LIB     := $(APPLICATION_DIR)/lib/lib$(APPLICATION_NAME)-$(METHOD).la
# application
app_EXEC    := $(APPLICATION_DIR)/$(APPLICATION_NAME)-$(METHOD)
# revision header
CAMEL_CASE_NAME := $(shell echo $(APPLICATION_NAME) | perl -pe 's/(?:^|_)([a-z])/\u$$1/g')
app_BASE_DIR    ?= base/
app_HEADER      := $(APPLICATION_DIR)/include/$(app_base_dir)$(CAMEL_CASE_NAME)Revision.h
# depend modules
depend_libs  := $(foreach i, $(DEPEND_MODULES), $(MOOSE_DIR)/modules/$(i)/lib/lib$(i)-$(METHOD).la)

##################################################################################################
# If we are NOT building a module, then make sure the dependency libs are updated to reflect
# all real dependencies
##################################################################################################
ifeq (,$(findstring $(APPLICATION_NAME), $(MODULE_NAMES)))
  depend_libs := $(depend_libs) $(app_LIBS)
endif
# Here we'll filter out MOOSE libs since we'll assume our application already has MOOSE compiled in
depend_libs := $(filter-out $(moose_LIBS),$(depend_libs))
# Create -L/-l versions of the depend libs
depend_libs_flags := $(foreach i, $(depend_libs), -L$(dir $(i)) -l$(shell echo $(notdir $(i)) | perl -pe 's/^lib(.*?)\.la/$$1/'))

# If building shared libs, make the plugins a dependency, otherwise don't.
ifeq ($(libmesh_shared),yes)
  app_plugin_deps := $(plugins)
else
  app_plugin_deps :=
endif

app_LIBS       := $(app_LIB) $(app_LIBS)
app_LIBS_other := $(filter-out $(app_LIB),$(app_LIBS))
app_HEADERS    := $(app_HEADER) $(app_HEADERS)
app_INCLUDES   += $(app_INCLUDE)
app_DIRS       += $(APPLICATION_DIR)

# dependencies
-include $(app_deps)

# Rest the DEPEND_MODULES variable in case this file is sourced again
DEPEND_MODULES :=

###############################################################################
# Build Rules:
#
###############################################################################
ifeq ($(BUILD_EXEC),yes)
  all:: $(app_EXEC)
endif

$(app_objects): $(app_HEADER)

# Target-specific Variable Values (See GNU-make manual)
$(app_HEADER): curr_dir    := $(APPLICATION_DIR)
$(app_HEADER): curr_app    := $(APPLICATION_NAME)
$(app_HEADER):
	@echo "MOOSE Generating Header "$@"..."
	$(shell $(FRAMEWORK_DIR)/scripts/get_repo_revision.py $(curr_dir) $@ $(curr_app))

# Target-specific Variable Values (See GNU-make manual)
$(app_LIB): curr_objs := $(app_objects)
$(app_LIB): curr_dir  := $(APPLICATION_DIR)
$(app_LIB): curr_deps := $(depend_libs)
$(app_LIB): curr_libs := $(depend_libs_flags)
$(app_LIB): $(app_objects) $(app_plugin_deps) $(app_HEADER) $(depend_libs)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(curr_objs) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(curr_dir)/lib $(curr_libs)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $@ $(curr_dir)/lib

$(app_EXEC): $(app_LIBS) $(mesh_library) $(main_object)
	@echo "Linking Executable "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(main_object) $(app_LIBS) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) $(ADDITIONAL_LIBS)

# Clang static analyzer
sa:: $(app_analyzer)
