# list all the files you need to compile

plugin_SOURCES := small_strain_tri_uel.cpp

# set plugin name
plugin_NAME := uel

# build the plugin (MOOSE_DIR must point to the root of the MOOSE installation)
-include $(MOOSE_DIR)/framework/plugin.mk

fail:
	@echo "Make sure the MOOSE_DIR environment variable is set and pointing to the root of the MOOSE installation."
