# list all the files you need to compile
plugin_SOURCES := umat.C src/build_stiffness_matrix.C

# list all include directories required by this plugin
plugin_INCLUDEDIRS :=  include

# optionally list include files here (or autogenerate a list like below)
# this will assist the build system dependency detection
plugin_INCLUDES := $(shell find $(plugin_INCLUDEDIRS) -name "*.h")

# build the plugin
-include $(MOOSE_DIR)/framework/plugin.mk
