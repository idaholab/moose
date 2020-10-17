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
MOOSE_DIR          ?= $(shell dirname `pwd`)/../moose
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################
CURRENT_DIR        := $(shell pwd)

# framework
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
include           $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# Extra stuff for GTEST
ADDITIONAL_INCLUDES	:= -I$(FRAMEWORK_DIR)/contrib/gtest
ADDITIONAL_LIBS 	:= $(FRAMEWORK_DIR)/contrib/gtest/libgtest.la

# dep apps
APPLICATION_DIR    := $(CURRENT_DIR)/..
APPLICATION_NAME   := darcy_thermo_mech
GEN_REVISION       := no
include            $(FRAMEWORK_DIR)/app.mk

APPLICATION_DIR    := $(CURRENT_DIR)
APPLICATION_NAME   := darcy_thermo_mech-unit
BUILD_EXEC         := yes
GEN_REVISION       := no
include $(FRAMEWORK_DIR)/app.mk

# Find all the DARCY_THERMO_MECH unit test source files and include their dependencies.
darcy_thermo_mech-unit_srcfiles := $(shell find $(CURRENT_DIR) -name "*.C")
darcy_thermo_mech-unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(darcy_thermo_mech-unit_srcfiles))
-include $(darcy_thermo_mech-unit_deps)

###############################################################################
# Additional special case targets should be added here
