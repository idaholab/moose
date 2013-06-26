###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
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
CURR_DIR  ?= $(shell pwd)
ROOT_DIR  ?= $(shell dirname `pwd`)

ifeq ($(MOOSE_DEV),true)
	MOOSE_DIR ?= $(ROOT_DIR)/devel/moose
else
	MOOSE_DIR ?= $(ROOT_DIR)/moose
endif

APPLICATION_NAME := moose_unit

DEP_APPS    ?= $(shell $(MOOSE_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))

# Extra stuff for CPPUNIT
CPPUNIT_DIR 		?= $(PACKAGES_DIR)/cppunit
ADDITIONAL_INCLUDES 	:= -I$(CPPUNIT_DIR)/include
ADDITIONAL_LIBS 	:= -L$(CPPUNIT_DIR)/lib -lcppunit 

include $(MOOSE_DIR)/build.mk
include $(MOOSE_DIR)/moose.mk
include $(MOOSE_DIR)/app.mk

# Find all the MOOSE unit test source files and include their dependencies.
moose_unit_srcfiles := $(shell find $(ROOT_DIR)/moose_unit -name "*.C")
moose_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(moose_unit_srcfiles))
-include $(moose_unit_deps)

###############################################################################
# Additional special case targets should be added here
