###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project 
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
MOOSE_DIR          ?= $(shell dirname `pwd`)
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

# dep apps
APPLICATION_DIR    := $(MOOSE_DIR)/test
APPLICATION_NAME   := moose_test
BUILD_EXEC         := yes
# Note: There are no applications that depend on moose_test so the test_up and up
# targets are not very useful here.  Instead we will test up like this is MOOSE
DEP_APPS           ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py MOOSE~$(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here

