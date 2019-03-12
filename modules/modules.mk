###############################################################################
############################ COMMON MODULES ###################################
###############################################################################

ifeq ($(ALL_MODULES),yes)
        CHEMICAL_REACTIONS          := yes
        CONTACT                     := yes
        FLUID_PROPERTIES            := yes
        FUNCTIONAL_EXPANSION_TOOLS  := yes
        HEAT_CONDUCTION             := yes
        LEVEL_SET                   := yes
        MISC                        := yes
        NAVIER_STOKES               := yes
        PHASE_FIELD                 := yes
        POROUS_FLOW                 := yes
        RDG                         := yes
        RICHARDS                    := yes
        SOLID_MECHANICS             := yes
        STOCHASTIC_TOOLS            := yes
        TENSOR_MECHANICS            := yes
        XFEM                        := yes
        EXTERNAL_PETSC_SOLVER       := yes
endif

ifeq ($(XFEM),yes)
        SOLID_MECHANICS             := yes
endif

ifeq ($(SOLID_MECHANICS),yes)
        TENSOR_MECHANICS            := yes
endif

ifeq ($(POROUS_FLOW),yes)
        TENSOR_MECHANICS            := yes
        FLUID_PROPERTIES            := yes
        CHEMICAL_REACTIONS          := yes
        RDG                         := yes
endif

ifeq ($(NAVIER_STOKES),yes)
        FLUID_PROPERTIES            := yes
        RDG                         := yes
        HEAT_CONDUCTION             := yes
endif

ifeq ($(PHASE_FIELD),yes)
        TENSOR_MECHANICS            := yes
endif

# The master list of all moose modules
MODULE_NAMES := "chemical_reactions contact fluid_properties functional_expansion_tools heat_conduction level_set misc navier_stokes phase_field porous_flow rdg richards solid_mechanics stochastic_tools tensor_mechanics xfem external_petsc_solver"

###############################################################################
########################## MODULE REGISTRATION ################################
###############################################################################
GEN_REVISION  := no

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

ifeq ($(FUNCTIONAL_EXPANSION_TOOLS),yes)
  APPLICATION_NAME   := functional_expansion_tools
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/$(APPLICATION_NAME)
  SUFFIX             := fet
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(HEAT_CONDUCTION),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/heat_conduction
  APPLICATION_NAME   := heat_conduction
  SUFFIX             := hc
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(LEVEL_SET),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/level_set
  APPLICATION_NAME   := level_set
  SUFFIX             := ls
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

  # Dependency on fluid properties and rdg
  DEPEND_MODULES     := fluid_properties rdg heat_conduction
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

ifeq ($(POROUS_FLOW),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/porous_flow
  APPLICATION_NAME   := porous_flow

  DEPEND_MODULES     := tensor_mechanics fluid_properties chemical_reactions rdg
  SUFFIX             := pflow
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

ifeq ($(XFEM),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/xfem
  APPLICATION_NAME   := xfem

  #Dependency on solid_mechanics
  DEPEND_MODULES     := solid_mechanics
  SUFFIX             := xfem
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(EXTERNAL_PETSC_SOLVER),yes)
  APPLICATION_NAME   := external_petsc_solver
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/${APPLICATION_NAME}

  SUFFIX             := eps
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

# Default to Generating revision for most applications
GEN_REVISION := yes
