###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Required Make variables
# APPLICATION_NAME  - the name of this application (all lower case)
# MOOSE_DIR	        - location of the MOOSE framework
#
# Optional Environment variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
#
# Note: Make sure that there is no whitespace after the word 'yes' if enabling
# an application
###############################################################################
CURR_DIR	?= $(shell pwd)
MOOSE_DIR	?= $(shell pwd)/../../moose

MAKE_LIBRARY := no
APPLICATION_NAME := ex01

include $(MOOSE_DIR)/build.mk
include $(MOOSE_DIR)/moose.mk
include ../test.mk

###############################################################################
# Additional special case targets should be added here

test: $(APPLICATION_NAME)-$(METHOD)
	$(call TEST_exodiff,2d_diffusion.i,out.e)
