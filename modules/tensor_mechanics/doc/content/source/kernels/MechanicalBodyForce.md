# MechanicalBodyForce

!syntax description /Kernels/MechanicalBodyForce

## Description

The kernel `MechanicalBodyForce` provides a body force term from a vector valued
material property in the stress divergence equilibrium. Kernels for all spatial
directions can be set up using the [MechanicalBodyForceAction.md].

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/mechanical_body_force/test.i block=Kernels/mechanical_body_force

!syntax parameters /Kernels/MechanicalBodyForce

!syntax inputs /Kernels/MechanicalBodyForce

!syntax children /Kernels/MechanicalBodyForce
