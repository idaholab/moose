# Saline
SALINE_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/saline
SALINE_SRC = $(SALINE_DIR)/src
ifneq ($(wildcard $(SALINE_SRC)/cpp/data_store.cc),)
  ADDITIONAL_CPPFLAGS += -DSALINE_ENABLED
  include $(APPLICATION_DIR)/contrib/saline.mk
endif
# Fluid properties submodules
AIR_FP_DIR                ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/air
AIR_FP_CONTENT            := $(shell ls $(AIR_FP_DIR) 2> /dev/null)
CARBON_DIOXIDE_FP_DIR     ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/carbon_dioxide
CARBON_DIOXIDE_FP_CONTENT := $(shell ls $(CARBON_DIOXIDE_FP_DIR) 2> /dev/null)
HELIUM_FP_DIR             ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/helium
HELIUM_FP_CONTENT         := $(shell ls $(HELIUM_FP_DIR) 2> /dev/null)
NITROGEN_FP_DIR           ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/nitrogen
NITROGEN_FP_CONTENT       := $(shell ls $(NITROGEN_FP_DIR) 2> /dev/null)
POTASSIUM_FP_DIR          ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/potassium
POTASSIUM_FP_CONTENT      := $(shell ls $(POTASSIUM_FP_DIR) 2> /dev/null)
SODIUM_FP_DIR             ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/sodium
SODIUM_FP_CONTENT         := $(shell ls $(SODIUM_FP_DIR) 2> /dev/null)

# Cache revision information
CAMEL_CASE_NAME_save := ${CAMEL_CASE_NAME}
app_BASE_DIR_save := ${app_BASE_DIR}
app_HEADER_save := ${app_HEADER}
GEN_REVISION_save := ${GEN_REVISION}

# AIR
ifneq ($(AIR_FP_CONTENT),)
ifneq ($(BUILDING_FP_APP), yes)
  AIR_DIR            := $(AIR_FP_DIR)
	APPLICATION_DIR    := $(AIR_FP_DIR)
	APPLICATION_NAME   := air
	libmesh_CXXFLAGS   += -DAIR_FP_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	DEPEND_MODULES     += fluid_properties
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(AIR_FP_DIR)/libSBTL_Air.mk
endif
endif

# CARBON_DIOXIDE
ifneq ($(CARBON_DIOXIDE_FP_CONTENT),)
ifneq ($(BUILDING_FP_APP), yes)
  CARBON_DIOXIDE_DIR := $(CARBON_DIOXIDE_FP_DIR)
	APPLICATION_DIR    := $(CARBON_DIOXIDE_FP_DIR)
	APPLICATION_NAME   := carbon_dioxide
	libmesh_CXXFLAGS   += -DCARBON_DIOXIDE_FP_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	DEPEND_MODULES     += fluid_properties
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(CARBON_DIOXIDE_FP_DIR)/libSBTL_CarbonDioxide.mk
endif
endif

# NITROGEN
ifneq ($(NITROGEN_FP_CONTENT),)
ifneq ($(BUILDING_FP_APP), yes)
	NITROGEN_DIR       := $(NITROGEN_FP_DIR)
	APPLICATION_DIR    := $(NITROGEN_FP_DIR)
	APPLICATION_NAME   := nitrogen
	libmesh_CXXFLAGS   += -DNITROGEN_FP_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	DEPEND_MODULES     += fluid_properties
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(NITROGEN_FP_DIR)/libSBTL_Nitrogen.mk
endif
endif

# HELIUM
ifneq ($(HELIUM_FP_CONTENT),)
ifneq ($(BUILDING_FP_APP), yes)
	HELIUM_DIR         := $(HELIUM_FP_DIR)
	APPLICATION_DIR    := $(HELIUM_FP_DIR)
	APPLICATION_NAME   := helium
	libmesh_CXXFLAGS   += -DHELIUM_FP_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	DEPEND_MODULES     += fluid_properties
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(HELIUM_FP_DIR)/libSBTL_Helium.mk
endif
endif

# POTASSIUM
ifneq ($(POTASSIUM_FP_CONTENT),)
ifneq ($(BUILDING_FP_APP), yes)
	POTASSIUM_DIR      := $(POTASSIUM_FP_DIR)
	APPLICATION_DIR    := $(POTASSIUM_FP_DIR)
	APPLICATION_NAME   := potassium
	libmesh_CXXFLAGS   += -DPOTASSIUM_FP_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	DEPEND_MODULES     += fluid_properties
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(POTASSIUM_FP_DIR)/libPotassiumProperties.mk
endif
endif

# SODIUM
ifneq ($(SODIUM_FP_CONTENT),)
ifneq ($(BUILDING_FP_APP), yes)
	SODIUM_DIR         := $(SODIUM_FP_DIR)
	APPLICATION_DIR    := $(SODIUM_FP_DIR)
	APPLICATION_NAME   := sodium
	libmesh_CXXFLAGS   += -DSODIUM_FP_ENABLED -DSKIP_MODULE_LOAD
	GEN_REVISION       := yes
	DEPEND_MODULES     += fluid_properties
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(SODIUM_FP_DIR)/libSodiumProperties.mk
endif
endif

# If building any fluid property submodule, we need to clear those before app.mk gets included
# for any other module, as those modules do not generate a revision
ifneq ($(AIR_FP_CONTENT)$(CARBON_DIOXIDE_FP_CONTENT)$(HELIUM_FP_CONTENT)$(NITROGEN_FP_CONTENT)$(POTASSIUM_FP_CONTENT)$(SODIUM_FP_CONTENT),)
  CAMEL_CASE_NAME := ${CAMEL_CASE_NAME_save}
	app_BASE_DIR := ${app_BASE_DIR_save}
	app_HEADER := ${app_HEADER_save}
	GEN_REVISION := ${GEN_REVISION_save}
endif
