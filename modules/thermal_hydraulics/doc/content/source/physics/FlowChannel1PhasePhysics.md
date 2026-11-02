# FlowChannel1PhasePhysics

This [Physics](syntax/Physics/index.md) object implements the equations for single-phase flow
(mass, momentum and energy) in a one-dimensional flow channel, with a reconstructed discontinuous
Galerkin (rDG) discretization. It is created by a [FlowChannel1PhaseAC.md], which gives it the
parameters of the flow channel, and is not declared in an input file.

It creates the variables of the equations, their initial conditions, the time derivative, wall
friction, area gradient and gravity kernels, the numerical flux between elements, and the
auxiliary variables of the fluid state, such as pressure and temperature. The wall friction and the
wall heat transfer coefficient are supplied by the closures objects of the flow channel.

!syntax parameters /Physics/ThermalHydraulics/FlowChannel1Phase/FlowChannel1PhasePhysics

!syntax inputs /Physics/ThermalHydraulics/FlowChannel1Phase/FlowChannel1PhasePhysics

!syntax children /Physics/ThermalHydraulics/FlowChannel1Phase/FlowChannel1PhasePhysics
