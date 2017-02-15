###############################################################################
############################ COMMON MODULES ###################################
###############################################################################

ifeq ($(ALL_MODULES),yes)
        CHEMICAL_REACTIONS        := yes
        CONTACT                   := yes
        FLUID_PROPERTIES          := yes
        HEAT_CONDUCTION           := yes
        MISC                      := yes
        NAVIER_STOKES             := yes
        PHASE_FIELD               := yes
        RDG                       := yes
        RICHARDS                  := yes
        SOLID_MECHANICS           := yes
        STOCHASTIC_TOOLS          := yes
        TENSOR_MECHANICS          := yes
        WATER_STEAM_EOS           := yes
        XFEM                      := yes
        POROUS_FLOW               := yes
        LEVEL_SET                 := yes
endif

ifeq ($(XFEM),yes)
        SOLID_MECHANICS           := yes
endif

ifeq ($(SOLID_MECHANICS),yes)
        TENSOR_MECHANICS          := yes
endif

ifeq ($(POROUS_FLOW),yes)
        TENSOR_MECHANICS          := yes
        FLUID_PROPERTIES          := yes
        CHEMICAL_REACTIONS        := yes
endif

ifeq ($(NAVIER_STOKES),yes)
        FLUID_PROPERTIES          := yes
endif

ifeq ($(PHASE_FIELD),yes)
        TENSOR_MECHANICS          := yes
endif

# The master list of all moose modules
MODULE_NAMES := "chemical_reactions contact fluid_properties heat_conduction linear_elasticity misc navier_stokes phase_field richards solid_mechanics tensor_mechanics water_steam_eos xfem porous_flow level_set"

###############################################################################
########################## MODULE REGISTRATION ################################
###############################################################################

ifeq ($(CHEMICAL_REACTIONS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/chemical_reactions
  APPLICATION_NAME   := chemical_reactions
  SUFFIX             := cr
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(CONTACT),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/contact
  APPLICATION_NAME   := contact
  SUFFIX             := con
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(FLUID_PROPERTIES),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/fluid_properties
  APPLICATION_NAME   := fluid_properties
  SUFFIX             := fp
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(HEAT_CONDUCTION),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/heat_conduction
  APPLICATION_NAME   := heat_conduction
  SUFFIX             := hc
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(MISC),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/misc
  APPLICATION_NAME   := misc
  SUFFIX             := misc
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(NAVIER_STOKES),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/navier_stokes
  APPLICATION_NAME   := navier_stokes

  # Dependency on porous flow
  DEPEND_MODULES     := fluid_properties
  SUFFIX             := ns
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(TENSOR_MECHANICS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/tensor_mechanics
  APPLICATION_NAME   := tensor_mechanics
  SUFFIX             := tm
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(PHASE_FIELD),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/phase_field
  APPLICATION_NAME   := phase_field

  # Dependency on tensor mechanics
  DEPEND_MODULES     := tensor_mechanics

  SUFFIX             := pf
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(RDG),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/rdg
  APPLICATION_NAME   := rdg
  SUFFIX             := rdg
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(RICHARDS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/richards
  APPLICATION_NAME   := richards
  SUFFIX             := rich
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(SOLID_MECHANICS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/solid_mechanics
  APPLICATION_NAME   := solid_mechanics

  #Dependency on tensor mechanics
  DEPEND_MODULES     := tensor_mechanics
  SUFFIX             := sm
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(STOCHASTIC_TOOLS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/stochastic_tools
  APPLICATION_NAME   := stochastic_tools
  SUFFIX             := st
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(WATER_STEAM_EOS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/water_steam_eos
  APPLICATION_NAME   := water_steam_eos
  SUFFIX             := ws
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(XFEM),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/xfem
  APPLICATION_NAME   := xfem

  #Dependency on solid_mechanics
  DEPEND_MODULES     := solid_mechanics
  SUFFIX             := xfem
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(POROUS_FLOW),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/porous_flow
  APPLICATION_NAME   := porous_flow

  #Dependency on tensor_mechanics, fluid_properties and chemical_reactions
  DEPEND_MODULES     := tensor_mechanics fluid_properties chemical_reactions
  SUFFIX             := pflow
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(LEVEL_SET),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/level_set
  APPLICATION_NAME   := level_set
  SUFFIX             := ls
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(ALL_MODULES),yes)
  ifneq ($(INCLUDE_COMBINED),no)
    APPLICATION_DIR    := $(MOOSE_DIR)/modules/combined
    APPLICATION_NAME   := combined
    SUFFIX             := comb
    include $(FRAMEWORK_DIR)/app.mk
  endif
endif

# The loader should be used for all applications. We
# only skip it when compiling individual modules
ifneq ($(SKIP_LOADER),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/module_loader
  APPLICATION_NAME   := module_loader
  LIBRARY_SUFFIX     := yes
  include $(FRAMEWORK_DIR)/app.mk
endif
