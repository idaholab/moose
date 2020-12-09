###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables (one of the following)
# PACKAGES_DIR  - Location of the MOOSE redistributable package
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
MOOSE_DIR          ?= $(shell dirname `pwd`)/../../../moose
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
# set desired physics modules equal to 'yes' to enable them
CHEMICAL_REACTIONS        := no
CONTACT                   := no
FLUID_PROPERTIES          := no
HEAT_CONDUCTION           := no
MISC                      := no
NAVIER_STOKES             := no
PHASE_FIELD               := no
RDG                       := no
RICHARDS                  := no
SOLID_MECHANICS           := no
STOCHASTIC_TOOLS          := no
TENSOR_MECHANICS          := no
WATER_STEAM_EOS           := no
XFEM                      := no
POROUS_FLOW               := no
LEVEL_SET                 := no
RAY_TRACING               := yes
include           $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# Extra stuff for GTEST
ADDITIONAL_INCLUDES	:= -I$(FRAMEWORK_DIR)/contrib/gtest
ADDITIONAL_LIBS 	:= $(FRAMEWORK_DIR)/contrib/gtest/libgtest.la

# dep apps
CURRENT_DIR        := $(shell pwd)
APPLICATION_DIR    := $(CURRENT_DIR)/..
APPLICATION_NAME   := ray_tracing
include            $(FRAMEWORK_DIR)/app.mk

APPLICATION_DIR    := $(CURRENT_DIR)
APPLICATION_NAME   := ray_tracing-unit
BUILD_EXEC         := yes

DEP_APPS    ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include $(FRAMEWORK_DIR)/app.mk

# Find all the RayTracing unit test source files and include their dependencies.
ray_tracing_unit_srcfiles := $(shell find $(MOOSE_DIR)/unit/src -name "*.C")
ray_tracing_unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(ray_tracing_unit_srcfiles))
-include $(ray_tracing_unit_deps)

###############################################################################
# Additional special case targets should be added here
