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
MOOSE_DIR          ?= $(shell dirname `pwd`)/moose
HERD_TRUNK_DIR     ?= $(shell dirname `pwd`)
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

CURR_DIR    := $(shell pwd)
ROOT_DIR    := $(HERD_TRUNK_DIR)

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
ALL_MODULES := yes
include           $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# dep apps
APPLICATION_DIR    := $(HERD_TRUNK_DIR)/r7_moose
APPLICATION_NAME   := r7_moose
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk

APPLICATION_DIR    := $(HERD_TRUNK_DIR)/raven
APPLICATION_NAME   := RAVEN

include $(HERD_TRUNK_DIR)/raven/config.mk
include $(HERD_TRUNK_DIR)/raven/raven.mk
include $(HERD_TRUNK_DIR)/raven/raven_python_modules.mk

###############################################################################
# Additional special case targets should be added here

