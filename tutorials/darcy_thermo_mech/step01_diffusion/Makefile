###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR        - Root directory of the MOOSE project
#
###############################################################################
# Use the MOOSE submodule if it exists and MOOSE_DIR is not set
MOOSE_SUBMODULE    := $(CURDIR)/moose
ifneq ($(wildcard $(MOOSE_SUBMODULE)/framework/Makefile),)
  MOOSE_DIR        ?= $(MOOSE_SUBMODULE)
else
  MOOSE_DIR        ?= $(shell dirname `pwd`)/../../../moose
endif

# framework
FRAMEWORK_DIR      := $(MOOSE_DIR)/framework
ADDITIONAL_CPPFLAGS += -Wall -Wextra
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
ALL_MODULES               := no

CHEMICAL_REACTIONS        := no
CONTACT                   := no
FLUID_PROPERTIES          := no
# heat_conduction is required by Steps 5+
HEAT_CONDUCTION           := yes
LINEAR_ELASTICITY         := no
MISC                      := no
NAVIER_STOKES             := no
# phase_field is required by Step 10.
PHASE_FIELD               := yes
POROUS_FLOW               := no
RICHARDS                  := no
# tensor_mechanics is required by Steps 9-10.
TENSOR_MECHANICS          := yes
XFEM                      := no
include $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := darcy_thermo_mech
BUILD_EXEC         := yes
GEN_REVISION       := no
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here
