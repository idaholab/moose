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
  MOOSE_DIR        ?= $(shell dirname `pwd`)/moose
endif

BISON_DIR ?= ""
BISON_CONTENT := $(shell ls $(BISON_DIR) 2> /dev/null)

# framework
FRAMEWORK_DIR      := $(MOOSE_DIR)/framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

################################## MODULES ####################################
# To use certain physics included with MOOSE, set variables below to
# yes as needed.  Or set ALL_MODULES to yes to turn on everything (overrides
# other set variables).

ALL_MODULES                 := no
CHEMICAL_REACTIONS          := no
CONTACT                     := no
EXTERNAL_PETSC_SOLVER       := no
FLUID_PROPERTIES            := yes
FUNCTIONAL_EXPANSION_TOOLS  := no
HEAT_CONDUCTION             := yes
LEVEL_SET                   := no
MISC                        := no
NAVIER_STOKES               := yes
PHASE_FIELD                 := no
POROUS_FLOW                 := no
RDG                         := no
REACTOR						:= yes
RICHARDS                    := no
SOLID_MECHANICS             := no
STOCHASTIC_TOOLS            := no
TENSOR_MECHANICS            := no
XFEM                        := no

ifneq ($(BISON_CONTENT),)
  TENSOR_MECHANICS  := yes
  CONTACT           := yes
  HEAT_CONDUCTION   := yes
  FLUID_PROPERTIES  := yes
  MISC              := yes
  PHASE_FIELD       := yes
  XFEM              := yes
  STOCHASTIC_TOOLS  := yes
  LEVEL_SET         := yes
endif

include $(MOOSE_DIR)/modules/modules.mk
###############################################################################

ifneq ($(BISON_CONTENT),)
  IAPWS95_SUBMODULE    := $(BISON_DIR)/iapws95
  ifneq ($(wildcard $(IAPWS95_SUBMODULE)/Makefile),)
    IAPWS95_DIR        ?= $(IAPWS95_SUBMODULE)
  else
    IAPWS95_DIR        ?= $(shell dirname `pwd`)/iapws95
  endif
  # check that IAPWS95 is available
  IAPWS95_CONTENT := $(shell ls $(IAPWS95_DIR) 2> /dev/null)
  ifeq ($(IAPWS95_CONTENT),)
    $(error IAPWS95 does not seem to be available. Make sure that either the submodule in BISON is checked out or that your IAPWS95_DIR points to the correct location)
  endif
  APPLICATION_DIR    := $(IAPWS95_DIR)
  APPLICATION_NAME   := iapws95
  include            $(FRAMEWORK_DIR)/app.mk
  include            $(IAPWS95_DIR)/libSBTL.mk

  libmesh_CXXFLAGS += -DBISON_ENABLED
  APPLICATION_DIR  := $(BISON_DIR)
  APPLICATION_NAME := bison
  include          $(FRAMEWORK_DIR)/app.mk
endif

# dep apps
APPLICATION_DIR    := $(CURDIR)
APPLICATION_NAME   := subchannel
BUILD_EXEC         := yes
GEN_REVISION       := no
ADDITIONAL_CPPFLAGS += -Wall -Wextra
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here

tags:
	@echo Building tags...
	@ctags -f .tags -R --c++-kinds=+p --fields=+iaS --extras=+q --exclude=doc --exclude=test --exclude=.git --exclude=problems --exclude="*header_symlinks*"
