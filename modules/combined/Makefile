###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project
# MODULE_DIR       - Location of the MOOSE modules directory
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
MODULE_DIR         ?= $(shell dirname `pwd`)
MOOSE_DIR          ?= $(shell dirname $(MODULE_DIR))
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ################################
ALL_MODULES       := yes
INCLUDE_COMBINED  := no
SKIP_LOADER       := yes
include $(MODULE_DIR)/modules.mk
###############################################################################

# Build the Module executable
APPLICATION_DIR    := $(MODULE_DIR)/combined
APPLICATION_NAME   := combined
BUILD_EXEC         := yes
# Note: There are no applications that depend on moose_test so the test_up and up
# targets are not very useful here.  Instead we will test up like this is MOOSE
DEP_APPS           ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here
