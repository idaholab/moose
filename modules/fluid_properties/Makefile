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

# Check that submodules are available
AIR_DIR                   ?= $(CURDIR)/contrib/air
AIR_CONTENT               := $(shell ls $(AIR_DIR) 2> /dev/null)
CARBON_DIOXIDE_DIR        ?= $(CURDIR)/contrib/carbon_dioxide
CARBON_DIOXIDE_CONTENT    := $(shell ls $(CARBON_DIOXIDE_DIR) 2> /dev/null)
HELIUM_DIR                ?= $(CURDIR)/contrib/helium
HELIUM_CONTENT            := $(shell ls $(HELIUM_DIR) 2> /dev/null)
NITROGEN_DIR              ?= $(CURDIR)/contrib/nitrogen
NITROGEN_CONTENT          := $(shell ls $(NITROGEN_DIR) 2> /dev/null)
POTASSIUM_DIR             ?= $(CURDIR)/contrib/potassium
POTASSIUM_CONTENT         := $(shell ls $(POTASSIUM_DIR) 2> /dev/null)
SODIUM_DIR                ?= $(CURDIR)/contrib/sodium
SODIUM_CONTENT            := $(shell ls $(SODIUM_DIR) 2> /dev/null)

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
ifneq ($(AIR_CONTENT),)
	AIR_DIR            ?= $(CURDIR)/contrib/air
	APPLICATION_DIR    := $(AIR_DIR)
	APPLICATION_NAME   := air
	libmesh_CXXFLAGS   += -DAIR_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(AIR_DIR)/libSBTL_Air.mk
endif

# CARBON_DIOXIDE
ifneq ($(CARBON_DIOXIDE_CONTENT),)
	CARBON_DIOXIDE_DIR ?= $(CURDIR)/contrib/carbon_dioxide
	APPLICATION_DIR    := $(CARBON_DIOXIDE_DIR)
	APPLICATION_NAME   := carbon_dioxide
	libmesh_CXXFLAGS   += -DCARBON_DIOXIDE_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(CARBON_DIOXIDE_DIR)/libSBTL_CarbonDioxide.mk
endif

# NITROGEN
ifneq ($(NITROGEN_CONTENT),)
	NITROGEN_DIR       ?= $(CURDIR)/contrib/nitrogen
	APPLICATION_DIR    := $(NITROGEN_DIR)
	APPLICATION_NAME   := nitrogen
	libmesh_CXXFLAGS   += -DNITROGEN_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(NITROGEN_DIR)/libSBTL_Nitrogen.mk
endif

# HELIUM
ifneq ($(HELIUM_CONTENT),)
	HELIUM_DIR         ?= $(CURDIR)/contrib/helium
	APPLICATION_DIR    := $(HELIUM_DIR)
	APPLICATION_NAME   := helium
	libmesh_CXXFLAGS   += -DHELIUM_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(HELIUM_DIR)/libSBTL_Helium.mk
endif

# SODIUM
ifneq ($(SODIUM_CONTENT),)
	SODIUM_DIR         ?= $(CURDIR)/contrib/sodium
	APPLICATION_DIR    := $(SODIUM_DIR)
	APPLICATION_NAME   := sodium
	libmesh_CXXFLAGS   += -DSODIUM_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(SODIUM_DIR)/libSodiumProperties.mk
endif

# POTASSIUM
ifneq ($(POTASSIUM_CONTENT),)
	POTASSIUM_DIR      ?= $(CURDIR)/contrib/potassium
	APPLICATION_DIR    := $(POTASSIUM_DIR)
	APPLICATION_NAME   := potassium
	libmesh_CXXFLAGS   += -DPOTASSIUM_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(POTASSIUM_DIR)/libPotassiumProperties.mk
endif
