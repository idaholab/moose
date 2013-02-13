###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
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
ROOT_DIR        ?= $(shell pwd)/../..

ifeq ($(MOOSE_DEV),true)
	MOOSE_DIR ?= $(ROOT_DIR)/devel/moose
else
	MOOSE_DIR ?= $(ROOT_DIR)/moose
endif

APPLICATION_NAME := ex17

include $(MOOSE_DIR)/build.mk
include $(MOOSE_DIR)/moose.mk
include $(MOOSE_DIR)/app.mk
include ../test.mk

# Include dependency files for this example
ex_srcfiles := $(shell find $(CURR_DIR) -name *.C)
ex_deps     := $(patsubst %.C, %.$(obj-suffix).d, $(ex_srcfiles))
-include $(ex_deps)

###############################################################################
# Additional special case targets should be added here

test: $(APPLICATION_NAME)-$(METHOD)
	$(call TEST_exodiff,ex17.i,out.e)
