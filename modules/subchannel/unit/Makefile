###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables (one of the following)
# PACKAGES_DIR  - Location of the MOOSE redistributable package
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
MOOSE_SUBMODULE    := $(CURDIR)/../moose
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/../moose
endif
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
# set desired physics modules equal to 'yes' to enable them
HEAT_CONDUCTION     := yes
NAVIER_STOKES       := yes
FLUID_PROPERTIES    := yes
MISC                := yes
THERMAL_HYDRAULICS  := yes
USE_TEST_LIBS       := yes
REACTOR             := yes

include           $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# Extra stuff for GTEST
ADDITIONAL_INCLUDES := -I$(FRAMEWORK_DIR)/contrib/gtest
ADDITIONAL_LIBS     := $(FRAMEWORK_DIR)/contrib/gtest/libgtest.la

# dep apps
CURRENT_DIR        := $(shell pwd)
APPLICATION_DIR    := $(CURRENT_DIR)/..
APPLICATION_NAME   := subchannel
include            $(FRAMEWORK_DIR)/app.mk

APPLICATION_DIR    := $(CURRENT_DIR)
APPLICATION_NAME   := subchannel-unit
BUILD_EXEC         := yes

DEP_APPS    ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include $(FRAMEWORK_DIR)/app.mk

# Find all the subchannel unit test source files and include their dependencies.
subchannel_unit_srcfiles := $(shell find $(CURRENT_DIR)/src -name "*.C")
subchannel_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(subchannel_unit_srcfiles))
-include $(subchannel_unit_deps)

###############################################################################
# Additional special case targets should be added here
