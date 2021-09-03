# MaterialVectorBodyForce

!syntax description /Kernels/MaterialVectorBodyForce

## Description

The kernel `MaterialVectorBodyForce` provides a body force term from a vector valued
material property in the stress divergence equilibrium. Kernels for all spatial
directions can be set up using the [MaterialVectorBodyForce action](MaterialVectorBodyForce/index.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/gravity/material_vector_body_force.i block=Modules/TensorMechanics/MaterialVectorBodyForce/all

!syntax parameters /Kernels/MaterialVectorBodyForce

!syntax inputs /Kernels/MaterialVectorBodyForce

!syntax children /Kernels/MaterialVectorBodyForce
