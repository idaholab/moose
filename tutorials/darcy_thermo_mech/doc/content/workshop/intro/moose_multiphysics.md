# Creating a Multiphysics Code

!---

- Multiphysics is popular, but how is it achieved?
- Scientists are adept at creating applications in their domain
- What about collaborating across research groups and/or disciplines?

  - Iterating between design teams?
  - Development of "coupling" codes?
  - Is there something better?

!---

## Modularity is Key

- Data should be accessed through strict interfaces with code having separation of responsibilities

  - Allows for "decoupling" of code
  - Leads to more reuse and less bugs
  - +Challenging for FEM+: Shape functions, DOFs, Elements, QPs, Material Properties, Analytic
    Functions, Global Integrals, Transferred Data and much more are needed in FEM assembly

    The complexity makes computational science codes brittle and hard to reuse

- A consistent set of "systems" are needed to carry out common actions, these systems should be
  separated by interfaces


!---

## MOOSE Pluggable Systems

- Systems break apart responsibility
- No direct communication between systems
- Everything flows through MOOSE interfaces
- Objects can be mixed and matched to achieve simulation goals
- Incoming data can be changed dynamically
- Outputs can be manipulated (e.g. multiplication by radius for cylindrical coordinates)
- An object, by itself, can be lifted from one application and used by another

!---

## MOOSE Pluggable Systems

!style! halign=center
!row!
!col! small=12 medium=6 large=3 style=margin-left:auto;margin-right:20px;
Actions\\
AuxKernels\\
Base\\
BCs\\
Constraints\\
Controls\\
Dampers\\
DGKernels\\
DiracKernels\\
Distributions\\
Executioners\\
!col-end!

!col! small=12 medium=6 large=3 style=margin-right:2%;
Functions\\
Geomsearch\\
ICs\\
Indicators\\
InterfaceKernels\\
Kernels\\
LineSearches\\
Markers\\
Materials\\
Mesh\\
MeshGenerators\\
!col-end!

!col! small=12 medium=6 large=3 style=margin-right:2%;
MeshModifiers\\
Multiapps\\
NodalKernels\\
Outputs\\
Parser\\
Partitioner\\
Positions\\
Postprocessors\\
Preconditioners\\
Predictors\\
Problems\\
!col-end!

!col! small=12 medium=6 large=3 style=margin-right:10%;
Reporters\\
RelationshipManagers\\
Samplers\\
Splits\\
TimeIntegrators\\
TimeSteppers\\
Transfers\\
UserObject\\
Utils\\
Variables\\
VectorPostprocessors\\
!col-end!
!row-end!
!style-end!

!---

## Finite-Element Reactor Fuel Simulation

!media darcy_thermo_mech/simulator.mp4 style=width:70%;margin-left:auto;margin-right:auto;display:block;

!---

## MOOSE Modules

!style! halign=center
!row!
!col! small=12 medium=6 large=3 style=margin-left:10%;
+Physics+\\
Chemical Reactions\\
Contact\\
Electromagnetics\\
Fluid Structure Interaction (FSI)\\
Geochemistry\\
Heat Transfer\\
Level Set\\
Navier Stokes\\
Peridynamics\\
Phase Field\\
Porous Flow\\
Solid Mechanics\\
Subchannel\\
Thermal Hydraulics\\
!col-end!

!col! small=12 medium=6 large=3 style=margin-right:2%;
+Numerics+\\
External PETSc Solver\\
Function Expansion Tools\\
Optimization\\
Ray Tracing\\
rDG\\
Stochastic Tools\\
XFEM\\
!col-end!

!col! small=12 medium=6 large=3 style=margin-right:10%;
+Physics support+\\
Fluid Properties\\
Solid Properties\\
Reactor\\
!col-end!
!row-end!
!style-end!

!---

## The MOOSE ecosystem

!media darcy_thermo_mech/moose_herd_2022.png style=margin-left:auto;margin-right:auto;display:block;

Many are open-source on GitHub. Some are accessible through the [NCRC](https://inl.gov/ncrc/)
