# Fluid-Structure Interaction Module

- [System Documentation List](fsi/systems.md)

The Fluid-Structure Interaction Module is a library of simulation tools that solve
fluid and structure problems, wherein, their behavior is inter-dependent. This module
provides a simple approach for implementing even advanced capabilities:

- Plug-n-play design enables users to incorporate the relevant physics for specific and varied simulations
- Straight-forward procedure for adding new physics

This module currently is capable of simulating fluid-structure interaction behavior
using an acoustic formulation for the fluid. More information about this physics is
 available [here](/fsi_acoustics.md). Efforts are underway to add more capabilities such as Arbitrary
Lagrangian Eulerian to this module.

## Explore the Capabilities

The +Fluid-Structure Interaction Module+ can be used in a variety of simulations
where the fluid and structural components are inter-dependent on each other. That is,
 the fluid behavior is affected by the structural behavior and vice-versa. The following
 figures show results from a few different simulations.

!row!

!media fsi/gravity_waves.png
       style=width:40%;float:left;padding-top:2.5%;
       caption=Gravity waves over a bed of fluid subjected to an initial disturbance. This was simulated using the [free surface boundary condition](/FluidFreeSurfaceBC.md).

!media fsi/tank.png
      style=width:35%;height:10%;float:right;padding-top:2.5%;
      caption=Steel tank with liquid nuclear fuel subjected to an external dynamic load. This was simulated using the [fluid-structure interface](/FluidStructureInterface.md).

!row-end!

## Developing New Fluid-Structure Interaction Code

Consider becoming a developer yourself. Follow the MOOSE standards for [contributing code and documentation](MooseDocs/generate.md optional=True).
