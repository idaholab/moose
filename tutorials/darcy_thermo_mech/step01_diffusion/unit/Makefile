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
SOLID_MECHANICS           := no
# tensor_mechanics is required by Steps 9-10.
TENSOR_MECHANICS          := yes
WATER_STEAM_EOS           := no
XFEM                      := no
include           $(MOOSE_DIR)/modules/modules.mk
###############################################################################

# Extra stuff for CPPUNIT
CPPUNIT_DIR 		?= $(PACKAGES_DIR)/cppunit
ADDITIONAL_INCLUDES 	:= -I$(CPPUNIT_DIR)/include
ADDITIONAL_LIBS 	:= -L$(CPPUNIT_DIR)/lib -lcppunit

# dep apps
APPLICATION_DIR    := $(CURRENT_DIR)/..
APPLICATION_NAME   := darcy_thermo_mech
include            $(FRAMEWORK_DIR)/app.mk

APPLICATION_DIR    := $(CURRENT_DIR)
APPLICATION_NAME   := darcy_thermo_mech-unit
BUILD_EXEC         := yes
DEP_APPS    ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include $(FRAMEWORK_DIR)/app.mk

# Find all the DARCY_THERMO_MECH unit test source files and include their dependencies.
darcy_thermo_mech-unit_srcfiles := $(shell find $(CURRENT_DIR) -name "*.C")
darcy_thermo_mech-unit_deps := $(patsubst %.C, %.$(obj-suffix).d, $(darcy_thermo_mech-unit_srcfiles))
-include $(darcy_thermo_mech-unit_deps)

###############################################################################
# Additional special case targets should be added here
