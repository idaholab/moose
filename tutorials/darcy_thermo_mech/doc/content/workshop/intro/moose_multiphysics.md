# Multiphysics Simulation

!---

## Historical Multiphysics Simulation

- Predictive multiphysics capability involved best-estimate calculations

  - Best estimates: data and correlation driven, many approximations
  - Necessitated experimental data for each design

- Physics performed independently

  - Was a "siloed" task; handoffs of data/results from person to person

- While individual codes were computationally efficient and well validated, coupling
  them was neither efficient nor well validated

- Used many approximations for evaluations of safety parameters

  - Pin-power reconstruction, gap conductance, spacer grid models

!---

## Modern Multiphysics Simulation

- Direct, physics-based models of all components

  - Reduces approximations as needed
  - Can be computationally expensive

- Can employ tighter and more consistent coupling

- Length and time scales of physics can be vastly different

  - What does this change for the analyst?

- Not as well validated:

  - Experiments prohibitively expensive
  - Very large design space

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

## Finite-Element Reactor Fuel Simulation

!media darcy_thermo_mech/simulator.mp4 alt=Video showing what is required to create a finite-element nuclear reactor fuel simulator, and then grouping those requirements based on what is within PETSc, libMesh, MOOSE, and example MOOSE-based applications. style=width:70%;margin-left:auto;margin-right:auto;display:block;

!---

## MOOSE Coupling Strategies

- Coupling strategies:

  - +Full coupling+: Solve all physics in a single (linear or nonlinear) system
  - +Loose coupling+: Solve each physics sequentially
  - +Tight coupling+: Solve each physics sequentially and iterate

- Segregated (loose/tight) coupling achieved using [`MultiApps`](MultiApps/index.md)

  - Physics coupled via in-memory transfer of fields and scalars

- Coupling specified in the input file (no code needed)

- No universally superior coupling strategy; correct choice depends on problem

- +Provides a standardized interface for an analyst to produce a coupled model+

!---

## MOOSE MultiApp Hierarchy Example

!style halign=center
!media multiapp_tree.png style=width:70% alt=Multiapp tree

