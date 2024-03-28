# Saline
SALINE_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/saline
SALINE_SRC = $(SALINE_DIR)/src
ifneq ($(wildcard $(SALINE_SRC)/cpp/data_store.cc),)
  ADDITIONAL_CPPFLAGS += -DSALINE_ENABLED
  include $(APPLICATION_DIR)/contrib/saline.mk
endif
# Fluid properties submodules
# AIR_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/air
# ifneq ($(wildcard $(AIR_DIR)/Makefile),)
#   ADDITIONAL_CPPFLAGS += -DAIR_ENABLED
#   ADDITIONAL_INCLUDES += -I$(AIR_SRC) -I$(AIR_DIR)/include
#   ADDITIONAL_LIBS += -L$(AIR_DIR) -lAir-$(METHOD)
#   ADDITIONAL_DEPEND_LIBS += $(Saline_LIB)
# endif
# CARBON_DIOXIDE_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/air
# ifneq ($(wildcard $(CARBON_DIOXIDE_DIR)/Makefile),)
#   ADDITIONAL_CPPFLAGS += -DCARBON_DIOXIDE_ENABLED
#   ADDITIONAL_INCLUDES += -I$(CARBON_DIOXIDE_SRC) -I$(CARBON_DIOXIDE_DIR)/include
#   ADDITIONAL_LIBS += -L$(CARBON_DIOXIDE_DIR) -lCarbonDioxide-$(METHOD)
#   ADDITIONAL_DEPEND_LIBS += $(Saline_LIB)
# endif
# HELIUM_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/air
# ifneq ($(wildcard $(HELIUM_DIR)/Makefile),)
#   ADDITIONAL_CPPFLAGS += -DHELIUM_ENABLED
# endif
# NITROGEN_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/air
# ifneq ($(wildcard $(NITROGEN_DIR)/Makefile),)
#   ADDITIONAL_CPPFLAGS += -DNITROGEN_ENABLED
# endif
# POTASSIUM_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/air
# ifneq ($(wildcard $(POTASSIUM_DIR)/Makefile),)
#   ADDITIONAL_CPPFLAGS += -DPOTASSIUM_ENABLED
# endif
# SODIUM_DIR ?= $(MOOSE_DIR)/modules/fluid_properties/contrib/air
# ifneq ($(wildcard $(SODIUM_DIR)/Makefile),)
#   ADDITIONAL_CPPFLAGS += -DSODIUM_ENABLED
# endif

AIR_DIR                   ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/air
AIR_CONTENT               := $(shell ls $(AIR_DIR) 2> /dev/null)
CARBON_DIOXIDE_DIR        ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/carbon_dioxide
CARBON_DIOXIDE_CONTENT    := $(shell ls $(CARBON_DIOXIDE_DIR) 2> /dev/null)
HELIUM_DIR                ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/helium
HELIUM_CONTENT            := $(shell ls $(HELIUM_DIR) 2> /dev/null)
NITROGEN_DIR              ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/nitrogen
NITROGEN_CONTENT          := $(shell ls $(NITROGEN_DIR) 2> /dev/null)
POTASSIUM_DIR             ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/potassium
POTASSIUM_CONTENT         := $(shell ls $(POTASSIUM_DIR) 2> /dev/null)
SODIUM_DIR                ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/sodium
SODIUM_CONTENT            := $(shell ls $(SODIUM_DIR) 2> /dev/null)

# AIR
ifneq ($(AIR_CONTENT),)
	AIR_DIR            ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/air
	APPLICATION_DIR    := $(AIR_DIR)
	APPLICATION_NAME   := air
	libmesh_CXXFLAGS   += -DAIR_ENABLED
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(AIR_DIR)/libSBTL_Air.mk
endif

# CARBON_DIOXIDE
ifneq ($(CARBON_DIOXIDE_CONTENT),)
	CARBON_DIOXIDE_DIR ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/carbon_dioxide
	APPLICATION_DIR    := $(CARBON_DIOXIDE_DIR)
	APPLICATION_NAME   := carbon_dioxide
	libmesh_CXXFLAGS   += -DCARBON_DIOXIDE_ENABLED
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(CARBON_DIOXIDE_DIR)/libSBTL_CarbonDioxide.mk
endif

# NITROGEN
ifneq ($(NITROGEN_CONTENT),)
	NITROGEN_DIR       ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/nitrogen
	APPLICATION_DIR    := $(NITROGEN_DIR)
	APPLICATION_NAME   := nitrogen
	libmesh_CXXFLAGS   += -DNITROGEN_ENABLED
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(NITROGEN_DIR)/libSBTL_Nitrogen.mk
endif

# HELIUM
ifneq ($(HELIUM_CONTENT),)
	HELIUM_DIR         ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/helium
	APPLICATION_DIR    := $(HELIUM_DIR)
	APPLICATION_NAME   := helium
	libmesh_CXXFLAGS   += -DHELIUM_ENABLED
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(HELIUM_DIR)/libSBTL_Helium.mk
endif

# SODIUM
ifneq ($(SODIUM_CONTENT),)
	SODIUM_DIR         ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/sodium
	APPLICATION_DIR    := $(SODIUM_DIR)
	APPLICATION_NAME   := sodium
	libmesh_CXXFLAGS   += -DSODIUM_ENABLED
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(SODIUM_DIR)/libSodiumProperties.mk
endif

# POTASSIUM
ifneq ($(POTASSIUM_CONTENT),)
	POTASSIUM_DIR      ?= ${MOOSE_DIR}/modules/fluid_properties/contrib/potassium
	APPLICATION_DIR    := $(POTASSIUM_DIR)
	APPLICATION_NAME   := potassium
	libmesh_CXXFLAGS   += -DPOTASSIUM_ENABLED
	include            $(FRAMEWORK_DIR)/app.mk
	include            $(POTASSIUM_DIR)/libPotassiumProperties.mk
endif
