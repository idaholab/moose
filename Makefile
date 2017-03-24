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
# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
MOOSE_SUBMODULE    := $(CURDIR)/moose
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/moose
endif
HERD_TRUNK_DIR     ?= $(shell dirname `pwd`)
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

CURR_DIR    := $(shell pwd)
ROOT_DIR    := $(HERD_TRUNK_DIR)

# framework
#include $(FRAMEWORK_DIR)/build.mk
#include $(FRAMEWORK_DIR)/moose.mk

APPLICATION_DIR    := $(HERD_TRUNK_DIR)/crow
APPLICATION_NAME   := CROW

include $(HERD_TRUNK_DIR)/crow/config.mk
include $(HERD_TRUNK_DIR)/crow/crow.mk
include $(HERD_TRUNK_DIR)/crow/crow_python_modules.mk

###############################################################################
# Additional special case targets should be added here

