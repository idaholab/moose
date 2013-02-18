###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
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

ifeq ($(MOOSE_DEV),true)
	MOOSE_DIR ?= $(ROOT_DIR)/devel/moose
else
	MOOSE_DIR ?= $(ROOT_DIR)/moose
endif

APPLICATION_NAME := moose_test

# Note: There are no applications that depend on moose_test so the test_up and up
# targets are not very useful here.  Instead we will test up like this is MOOSE
DEP_APPS    ?= $(shell $(MOOSE_DIR)/scripts/find_dep_apps.py MOOSE~$(APPLICATION_NAME))

include $(MOOSE_DIR)/build.mk
include $(MOOSE_DIR)/moose.mk
include $(MOOSE_DIR)/app.mk

# Include dependency files for moose_test
moose_test_srcfiles := $(shell find $(CURR_DIR) -name *.C)
moose_test_deps     := $(patsubst %.C, %.$(obj-suffix).d, $(moose_test_srcfiles))
-include $(moose_test_deps)
