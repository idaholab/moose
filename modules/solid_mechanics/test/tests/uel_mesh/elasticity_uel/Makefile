# list all the files you need to compile

plugin_SOURCES := constants.f90 gauss_quadrature.f90 matrix_operations.f90 physics_equations.f90 shape_functions.f90 solve_subs.f90 uel.f90 umat.f90

# set plugin name
plugin_NAME := uel

# build the plugin (MOOSE_DIR must point to the root of the MOOSE installation)
MOOSE_DIR ?= ../../../../../..
-include $(MOOSE_DIR)/framework/plugin.mk

fail:
	@echo "Make sure the MOOSE_DIR environment variable is set and pointing to the root of the MOOSE installation."
