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
MOOSE_DIR          ?= $(shell dirname `pwd`)
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
ADDITIONAL_CPPFLAGS += -Wall -Wextra
###############################################################################

# Extra stuff for GTEST
ADDITIONAL_INCLUDES	:= -isystem $(FRAMEWORK_DIR)/contrib/gtest
ADDITIONAL_LIBS 	:= $(FRAMEWORK_DIR)/contrib/gtest/libgtest.la

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

APPLICATION_DIR  := $(MOOSE_DIR)/unit
APPLICATION_NAME := moose-unit
BUILD_EXEC       := yes
app_BASE_DIR     :=      # Intentionally blank
DEP_APPS    ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include $(FRAMEWORK_DIR)/app.mk

# Find all the MOOSE unit test source files and include their dependencies.
moose_unit_srcfiles := $(shell find $(MOOSE_DIR)/unit/src -name "*.C")
moose_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(moose_unit_srcfiles))
-include $(moose_unit_deps)

###############################################################################
# Additional special case targets should be added here
