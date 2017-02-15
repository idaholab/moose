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
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

# dependency on rDG module
RDG              := yes

# dependency on fluid_properties module
FLUID_PROPERTIES := yes
SKIP_LOADER      := yes
include $(MODULE_DIR)/modules.mk

# dep apps
APPLICATION_DIR    := $(MODULE_DIR)/navier_stokes
APPLICATION_NAME   := navier_stokes
BUILD_EXEC         := yes
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here
