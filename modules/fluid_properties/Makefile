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
MODULE_NAME := fluid_properties
include $(MODULE_DIR)/modules.mk

###############################################################################
# Additional special case targets should be added here

# AIR
AIR_DIR            ?= $(CURDIR)/contrib/air
APPLICATION_DIR    := $(AIR_DIR)
APPLICATION_NAME   := air
include            $(FRAMEWORK_DIR)/app.mk
include            $(AIR_DIR)/libSBTL_Air.mk

# CARBON_DIOXIDE
CARBON_DIOXIDE_DIR ?= $(CURDIR)/contrib/carbon_dioxide
APPLICATION_DIR    := $(CARBON_DIOXIDE_DIR)
APPLICATION_NAME   := carbon_dioxide
include            $(FRAMEWORK_DIR)/app.mk
include            $(CARBON_DIOXIDE_DIR)/libSBTL_CarbonDioxide.mk

# NITROGEN
NITROGEN_DIR       ?= $(CURDIR)/contrib/nitrogen
APPLICATION_DIR    := $(NITROGEN_DIR)
APPLICATION_NAME   := nitrogen
include            $(FRAMEWORK_DIR)/app.mk
include            $(NITROGEN_DIR)/libSBTL_Nitrogen.mk

# HELIUM
HELIUM_DIR         ?= $(CURDIR)/contrib/helium
APPLICATION_DIR    := $(HELIUM_DIR)
APPLICATION_NAME   := helium
include            $(FRAMEWORK_DIR)/app.mk
include            $(HELIUM_DIR)/libSBTL_Helium.mk

# SODIUM
SODIUM_DIR         ?= $(CURDIR)/contrib/sodium
APPLICATION_DIR    := $(SODIUM_DIR)
APPLICATION_NAME   := sodium
include            $(FRAMEWORK_DIR)/app.mk
include            $(SODIUM_DIR)/libSodiumProperties.mk

# POTASSIUM
POTASSIUM_DIR      ?= $(CURDIR)/contrib/potassium
APPLICATION_DIR    := $(POTASSIUM_DIR)
APPLICATION_NAME   := potassium
include            $(FRAMEWORK_DIR)/app.mk
include            $(POTASSIUM_DIR)/libPotassiumProperties.mk
