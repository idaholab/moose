###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Required Make variables
# APPLICATION_NAME  - the name of this application (all lower case)
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Optional Environment variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
#
# Note: Make sure that there is no whitespace after the word 'yes' if enabling
# an application
###############################################################################
CURR_DIR  ?= $(shell pwd)
ROOT_DIR  ?= $(shell dirname `pwd`)
MOOSE_DIR ?= $(ROOT_DIR)/moose


MAKE_LIBRARY := no
APPLICATION_NAME := moose_test
include $(MOOSE_DIR)/build.mk

# Include the MOOSE Export file
include $(MOOSE_DIR)/moose.mk

###############################################################################
# Additional special case targets should be added here
