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

# Module
MODULE_NAME := ray_tracing
include $(MODULE_DIR)/modules.mk

# TODO: find a better way to enable this
# ADDITIONAL_CPPFLAGS += -DSINGLE_PRECISION_RAY

###############################################################################
# Additional special case targets should be added here
