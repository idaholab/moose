###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
# HERD_TRUNK_DIR   - Location of the HERD repository
# FRAMEWORK_DIR    - Location of the MOOSE framework
#
###############################################################################
MODULE_DIR         ?= $(shell dirname `pwd`)
MOOSE_DIR          ?= $(shell dirname $(MODULE_DIR))
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
ADDITIONAL_CPPFLAGS += -Wall -Wextra
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

MOOSE_TEST := yes
ifeq ($(MOOSE_TEST),yes)
  APPLICATION_NAME   := moose_test
  APPLICATION_DIR    := $(MOOSE_DIR)/test
  SUFFIX             := mt
  include $(FRAMEWORK_DIR)/app.mk
endif

# Module
MODULE_NAME := stochastic_tools
include $(MODULE_DIR)/modules.mk
