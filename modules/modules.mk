################################################################################
############################ COMMON MODULES ####################################
################################################################################

# When building an individual module, you should define MODULE_NAME (lower case)
# to the module you want to build before including this file. When doing that,
# there is no need to duplicate the dependencies in the individual module's
# Makefile.

ifneq (,$(MODULE_NAME))
  # Exec will automatically be built for the given MODULE_NAME
  SKIP_LOADER := yes
  UC_APP = $(shell echo $(MODULE_NAME) | tr a-z A-Z)
  $(eval $(UC_APP):=yes)
endif

ifeq ($(ALL_MODULES),yes)
        CHEMICAL_REACTIONS          := yes
        CONTACT                     := yes
        ELECTROMAGNETICS            := yes
        EXTERNAL_PETSC_SOLVER       := yes
        FLUID_PROPERTIES            := yes
        FSI                         := yes
        FUNCTIONAL_EXPANSION_TOOLS  := yes
        GEOCHEMISTRY                := yes
        HEAT_CONDUCTION             := yes
        LEVEL_SET                   := yes
        MISC                        := yes
        OPTIMIZATION                := yes
        NAVIER_STOKES               := yes
        PERIDYNAMICS                := yes
        PHASE_FIELD                 := yes
        POROUS_FLOW                 := yes
        RAY_TRACING                 := yes
        RDG                         := yes
        REACTOR                     := yes
        RICHARDS                    := yes
        SCALAR_TRANSPORT            := yes
        SOLID_PROPERTIES            := yes
        STOCHASTIC_TOOLS            := yes
        TENSOR_MECHANICS            := yes
        THERMAL_HYDRAULICS          := yes
        XFEM                        := yes
endif

# Modules that follow have one or more dependencies
# on the modules defined above them.
ifeq ($(THERMAL_HYDRAULICS),yes)
        NAVIER_STOKES               := yes
        FLUID_PROPERTIES            := yes
        HEAT_CONDUCTION             := yes
        RAY_TRACING                 := yes
        RDG                         := yes
        SOLID_PROPERTIES            := yes
        MISC                        := yes
endif

ifeq ($(FSI),yes)
        NAVIER_STOKES               := yes
        TENSOR_MECHANICS            := yes
endif

ifeq ($(NAVIER_STOKES),yes)
        FLUID_PROPERTIES            := yes
        HEAT_CONDUCTION             := yes
        RDG                         := yes
endif

ifeq ($(SOLID_PROPERTIES),yes)
        HEAT_CONDUCTION             := yes
endif

ifeq ($(CONTACT),yes)
        TENSOR_MECHANICS            := yes
endif

ifeq ($(HEAT_CONDUCTION),yes)
        RAY_TRACING                 := yes
endif

ifeq ($(PERIDYNAMICS),yes)
        TENSOR_MECHANICS            := yes
endif

ifeq ($(PHASE_FIELD),yes)
        TENSOR_MECHANICS            := yes
endif

ifeq ($(POROUS_FLOW),yes)
        CHEMICAL_REACTIONS          := yes
        FLUID_PROPERTIES            := yes
        TENSOR_MECHANICS            := yes
endif

ifeq ($(XFEM),yes)
        TENSOR_MECHANICS            := yes
endif

ifeq ($(SCALAR_TRANSPORT),yes)
        CHEMICAL_REACTIONS          := yes
        NAVIER_STOKES               := yes
        THERMAL_HYDRAULICS          := yes
        FLUID_PROPERTIES            := yes
        HEAT_CONDUCTION             := yes
        RDG                         := yes
        RAY_TRACING                 := yes
        SOLID_PROPERTIES            := yes
        MISC                        := yes
endif

# The master list of all moose modules
MODULE_NAMES := "chemical_reactions contact electromagnetics external_petsc_solver fluid_properties fsi functional_expansion_tools geochemistry heat_conduction level_set misc navier_stokes optimization peridynamics phase_field porous_flow ray_tracing rdg reactor richards scalar_transport solid_properties stochastic_tools tensor_mechanics thermal_hydraulics xfem"

################################################################################
########################## MODULE REGISTRATION #################################
################################################################################
GEN_REVISION  := no

# The ordering of the following is important! This is processed from the top down,
# therefore any modules with dependencies must have their dependent module's
# application(s) defined first.

# The modules that follow do not have any dependencies, so they're just
# ordered alphabetically.

ifeq ($(CHEMICAL_REACTIONS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/chemical_reactions
  APPLICATION_NAME   := chemical_reactions
  SUFFIX             := cr
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(ELECTROMAGNETICS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/electromagnetics
  APPLICATION_NAME   := electromagnetics
  SUFFIX             := em
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(EXTERNAL_PETSC_SOLVER),yes)
  APPLICATION_NAME   := external_petsc_solver
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/${APPLICATION_NAME}
  SUFFIX             := eps
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

ifeq ($(GEOCHEMISTRY),yes)
  APPLICATION_NAME   := geochemistry
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/$(APPLICATION_NAME)
  SUFFIX             := gc
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(LEVEL_SET),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/level_set
  APPLICATION_NAME   := level_set
  SUFFIX             := ls
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(RAY_TRACING),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/ray_tracing
  APPLICATION_NAME   := ray_tracing
  SUFFIX             := ray
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(RDG),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/rdg
  APPLICATION_NAME   := rdg
  SUFFIX             := rdg
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(REACTOR),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/reactor
  APPLICATION_NAME   := reactor
  SUFFIX             := rct
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(RICHARDS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/richards
  APPLICATION_NAME   := richards
  SUFFIX             := rich
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(STOCHASTIC_TOOLS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/stochastic_tools
  APPLICATION_NAME   := stochastic_tools
  SUFFIX             := st
  include $(FRAMEWORK_DIR)/app.mk
endif

# The modules that follow are purposefully ordered such that all of their
# dependencies are defined first

# Depended on by navier_stokes, fsi (through navier_stokes)
ifeq ($(HEAT_CONDUCTION),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/heat_conduction
  APPLICATION_NAME   := heat_conduction
  DEPEND_MODULES     := ray_tracing
  SUFFIX             := hc
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(SOLID_PROPERTIES),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/solid_properties
  APPLICATION_NAME   := solid_properties
  DEPEND_MODULES     := heat_conduction
  SUFFIX             := sp
  include $(FRAMEWORK_DIR)/app.mk
endif

# Dependend on by contact, fsi, misc, peridynamics, phase_field, porous_flow, xfem
ifeq ($(TENSOR_MECHANICS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/tensor_mechanics
  APPLICATION_NAME   := tensor_mechanics
  SUFFIX             := tm
  include $(FRAMEWORK_DIR)/app.mk
endif

# Depended on by fsi
ifeq ($(NAVIER_STOKES),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/navier_stokes
  APPLICATION_NAME   := navier_stokes
  DEPEND_MODULES     := fluid_properties rdg heat_conduction
  SUFFIX             := ns
  include $(FRAMEWORK_DIR)/app.mk
endif

# The following have their dependencies defined above and do not have any
# dependers, so we're back to alphabetical.

ifeq ($(CONTACT),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/contact
  APPLICATION_NAME   := contact
  DEPEND_MODULES     := tensor_mechanics
  SUFFIX             := con
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(FSI),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/fsi
  APPLICATION_NAME   := fsi
  DEPEND_MODULES     := navier_stokes tensor_mechanics
  SUFFIX             := fsi
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(MISC),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/misc
  APPLICATION_NAME   := misc
  SUFFIX             := misc
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(OPTIMIZATION),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/optimization
  APPLICATION_NAME   := optimization
  SUFFIX             := opt
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(PERIDYNAMICS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/peridynamics
  APPLICATION_NAME   := peridynamics
  DEPEND_MODULES     := tensor_mechanics
  SUFFIX             := pd
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(PHASE_FIELD),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/phase_field
  APPLICATION_NAME   := phase_field
  DEPEND_MODULES     := tensor_mechanics
  SUFFIX             := pf
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(POROUS_FLOW),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/porous_flow
  APPLICATION_NAME   := porous_flow
  DEPEND_MODULES     := tensor_mechanics fluid_properties chemical_reactions
  SUFFIX             := pflow
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(THERMAL_HYDRAULICS),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/thermal_hydraulics
  APPLICATION_NAME   := thermal_hydraulics
  DEPEND_MODULES     := navier_stokes fluid_properties heat_conduction rdg ray_tracing solid_properties misc
  SUFFIX             := th
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(SCALAR_TRANSPORT),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/scalar_transport
  APPLICATION_NAME   := scalar_transport
  DEPEND_MODULES     := chemical_reactions navier_stokes thermal_hydraulics fluid_properties heat_conduction rdg ray_tracing solid_properties misc
  SUFFIX             := st
  include $(FRAMEWORK_DIR)/app.mk
endif

ifeq ($(XFEM),yes)
  APPLICATION_DIR    := $(MOOSE_DIR)/modules/xfem
  APPLICATION_NAME   := xfem
  DEPEND_MODULES     := tensor_mechanics
  SUFFIX             := xfem
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
