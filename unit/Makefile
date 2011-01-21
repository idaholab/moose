###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Optional Environment variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Required Make variables
# APP_NAME	- the name of this application (all lower case)
# ENABLE_ELK 	- should be set to 'yes' to enable ELK
#
# Note: Make sure that there is no whitespace after the word 'yes' if enabling
# an application
###############################################################################
CURR_DIR	?= $(shell pwd)
ROOT_DIR  ?= $(shell dirname `pwd`)
MOOSE_DIR	?= $(ROOT_DIR)/moose

MAKE_LIBRARY := no
APPLICATION_NAME := moose_unit

# Extra stuff for CPPUNIT
CPPUNIT_DIR 		?= $(PACKAGES_DIR)/cppunit
ADDITIONAL_INCLUDES 	:= -I$(CPPUNIT_DIR)/include
ADDITIONAL_LIBS 	:= -L$(CPPUNIT_DIR)/lib -lcppunit 

include $(MOOSE_DIR)/build.mk

# Include the MOOSE Export file
include $(MOOSE_DIR)/moose.mk

###############################################################################
# Additional special case targets should be added here
