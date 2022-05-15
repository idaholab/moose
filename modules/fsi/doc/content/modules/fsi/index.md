# Fluid-Structure Interaction Module

- [System Documentation List](fsi/systems.md)

The Fluid-Structure Interaction Module is a library of simulation tools that solve
fluid and structure problems, wherein, their behavior is inter-dependent. This module
provides a simple approach for implementing even advanced capabilities:

- Plug-n-play design enables users to incorporate the relevant physics for specific and varied simulations
- Straight-forward procedure for adding new physics

This module currently is capable of simulating fluid-structure interaction behavior
using an acoustic formulation for the fluid. More information about this physics is
 available [here](/fsi_acoustics.md) and in [!cite](dhulipala2022acousticfsi).
Efforts are underway to add more capabilities such as Arbitrary Lagrangian Eulerian to this module.

## Explore the Capabilities

The +Fluid-Structure Interaction Module+ can be used in a variety of simulations
where the fluid and structural components are inter-dependent on each other. That is,
 the fluid behavior is affected by the structural behavior and vice-versa.

## Developing New Fluid-Structure Interaction Code

Consider becoming a developer yourself. Follow the MOOSE standards for [contributing code and documentation](MooseDocs/generate.md optional=True).
