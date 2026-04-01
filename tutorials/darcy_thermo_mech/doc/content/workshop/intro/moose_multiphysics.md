# Multiphysics Simulation

!---

## Historical Multiphysics Simulation

- Predictive multiphysics capability involved best-estimate calculations

  - Best estimates: data and correlation driven, many approximations
  - Necessitated experimental data for each design

- Physics performed independently

  - Was a "siloed" task; handoffs of data/results from person to person

- Codes are very computationally efficient and well validated

  - ... but the process wasn't necessarily efficient

- Used many approximations for evaluations of safety parameters

  - Pin-power reconstruction, gap conductance, spacer grid models

!---

## Current Multiphysics Simulation

- Direct, physics-based models of all components

  - Reduces approximations as needed
  - Can be computationally expensive
  - Validation?

- Tighter and consistent coupling

  - How separate - full coupling or loose coupling?

- Length and time scales of physics can be vastly different

  - What does this change for the analyst?

- Quite new: +validation of direct physics-based models are not as well validated+

!--

## Writing a Multiphysics Code

- Scientists are adept at creating applications in their domain
- What about collaborating across research groups and/or disciplines?

  - Iterating between design teams?
  - Development of "coupling" codes?

!---

## Modularity is Key

- Data should be accessed through strict interfaces with code having separation of responsibilities

  - Allows for "decoupling" of code
  - Leads to more reuse and less bugs
  - +Challenging for FEM+

    - Shape functions, degrees of freedom, meshing, quadrature points, material properties, analytic functions, global integrals, data transfer, ...
    - The complexity makes computational science codes brittle and hard to reuse

- A consistent set of "systems" are needed to carry out common actions, these systems should be
  separated by interfaces


!---

## MOOSE Pluggable Systems

- Systems break apart responsibility
- No direct communication between systems
- Data flows through MOOSE interfaces
- Objects can be mixed and matched to achieve simulation goals
- An object, by itself, can be lifted from one application and used by another


!---

## Finite-Element Reactor Fuel Simulation

!media darcy_thermo_mech/simulator.mp4 alt=Video showing what is required to create a finite-element nuclear reactor fuel simulator, and then grouping those requirements based on what is within PETSc, libMesh, MOOSE, and example MOOSE-based applications. style=width:70%;margin-left:auto;margin-right:auto;display:block;

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

## MOOSE Coupling Strategy

- Numerically separates a multiphysics solve by physics components (equations)

  - Connect physics together via in-memory transfer of fields and scalars

- Input file based

  - No code needed!

- There is no single unique hierarchy to solve a specific problem

  - Multiple may exist, some may be more efficient

- +Provides a standardized interface for an analyst to produce a coupled model+

!---

!style halign=center
!media tutorials/darcy_thermo_mech/multiapp_hierarchy.png style=width:70% alt=Example of multi-level MultiApp hierarchy.


!---

## The MOOSE ecosystem

!media darcy_thermo_mech/moose_herd_2022.png alt=The MOOSE herd, as of 2022. style=margin-left:auto;margin-right:auto;display:block;

Many are open-source on GitHub. Some are accessible through the [NCRC](https://inl.gov/ncrc/)
