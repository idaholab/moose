###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
# MODULE_DIR       - Location of the MOOSE modules directory
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

################################## SALINE #######################################
# Include link to Saline property database if the directory exists
ifneq ("$(wildcard saline_moose/Makefile)", "")
   #MOOSE_UNITY=false
   APPLICATION_DIR    := $(shell pwd)/saline_moose
   APPLICATION_NAME   := saline_moose
   BUILD_EXEC         := no
   ADDITIONAL_CPPFLAGS += -DSALINE_ENABLED
   ADDITIONAL_INCLUDES := -I$(APPLICATION_DIR)/include/base
   ADDITIONAL_LIBS    += -L$(APPLICATION_DIR)/lib
   DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
   include            $(FRAMEWORK_DIR)/app.mk
   #MOOSE_UNITY=true
endif

# Module
MODULE_NAME := fluid_properties
include $(MODULE_DIR)/modules.mk

###############################################################################
# Additional special case targets should be added here


