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
LIBMESH_DIR	?= $(shell pwd)/../../libmesh

APPLICATION_NAME := ex10

include $(MOOSE_DIR)/build.mk
include $(MOOSE_DIR)/moose.mk
include $(MOOSE_DIR)/app.mk
include ../test.mk

###############################################################################
# Additional special case targets should be added here

test: $(APPLICATION_NAME)-$(METHOD)
	$(call TEST_exodiff,ex10.i,out.e)
