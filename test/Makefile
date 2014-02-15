###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Make variables
# APPLICATION_NAME  - the name of this application (all lower case)
# CURR_DIR	    - current directory (DO NOT MODIFY THIS VARIABLE)
#
# Optional Environment variables
# MOOSE_DIR	    - location of the MOOSE framework
###############################################################################
APPLICATION_NAME := moose_test
CURR_DIR         ?= $(shell pwd)

MOOSE_DIR	 ?= $(shell dirname `pwd`)

# Note: There are no applications that depend on moose_test so the test_up and up
# targets are not very useful here.  Instead we will test up like this is MOOSE
MOOSE_TEST_DIR  := $(MOOSE_DIR)/test
DEP_APPS  := $(shell $(MOOSE_DIR)/framework/scripts/find_dep_apps.py MOOSE~$(APPLICATION_NAME))

###############################################################################
include $(MOOSE_DIR)/framework/build.mk
include $(MOOSE_DIR)/framework/moose.mk
include $(MOOSE_TEST_DIR)/moose_test.mk

###############################################################################
# Additional special case targets should be added here

