#
# Build system component for multi-file plugins (e.g. UMAT, UEXTERNALDB)
# Check modules/tensor_mechanics/examples/umat_build/Makefile for an example use
#

# make sure MOOSE_DIR is set
ifeq (x$(MOOSE_DIR),x)
  $(error Set the MOOSE_DIR environment variable to point to the root of the Moose repository.)
endif

# If the user has no environment variable
# called METHOD, they get optimized mode.
ifeq (x$(METHOD),x)
  METHOD := opt
endif

-include $(MOOSE_DIR)/framework/build.mk

ADDITIONAL_CPPFLAGS += $(foreach i, $(shell find $(plugin_INCLUDEDIRS) $(MOOSE_DIR)/modules/tensor_mechanics/include/utils/abaqus -type d), -I $(i))

plugin_OBJECTS := $(patsubst %.C, %.$(obj-suffix), $(filter %.C, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.cpp, %.$(obj-suffix), $(filter %.cpp, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.cc, %.$(obj-suffix), $(filter %.cc, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.c, %.$(obj-suffix), $(filter %.c, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.f, %.$(obj-suffix), $(filter %.f, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.F, %.$(obj-suffix), $(filter %.F, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.f90, %.$(obj-suffix), $(filter %.f90, $(plugin_SOURCES)))
plugin_OBJECTS += $(patsubst %.mod, %.$(obj-suffix), $(filter %.mod, $(plugin_SOURCES)))

plugin_NAME ?= umat
plugin_FILE := $(plugin_NAME)-$(METHOD).plugin
plugin_LAFILE := lib$(plugin_NAME)-$(METHOD).la
plugin_SOFILE = $(shell . ./$(plugin_LAFILE) && echo $$dlname)

all: $(plugin_FILE)

# we use libtool to link and temporarily "install" the plugin library
$(plugin_LAFILE) : $(plugin_OBJECTS) $(plugin_INCLUDES)
	@echo "Linking plugin "$(plugin_NAME)"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) -o $@ $(plugin_OBJECTS) $(libmesh_LDFLAGS) $(libmesh_LIBS) $(EXTERNAL_FLAGS) -rpath $(CURDIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(plugin_LAFILE) $(CURDIR)

# here we copy the lib*.so file to the expected plugin name and uninstall the libtool clutter right away
$(plugin_FILE): $(plugin_LAFILE)
	@install -c $(plugin_SOFILE) $(plugin_FILE)
	@$(libmesh_LIBTOOL) --mode=uninstall --quiet rm -f $(plugin_LAFILE)

clean:
	@$(libmesh_LIBTOOL) --mode=uninstall --quiet rm -f $(plugin_LAFILE)
	@rm -rf $(plugin_OBJECTS) $(plugin_FILE) .libs
