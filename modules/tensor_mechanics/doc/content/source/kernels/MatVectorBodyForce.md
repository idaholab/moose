# MatVectorBodyForce

!syntax description /Kernels/MatVectorBodyForce

## Description

The kernel `MatVectorBodyForce` provides a body force term from a vector valued
material property in the stress divergence equilibrium. Kernels for all spatial
directions can be set up using the [MatVectorBodyForce action](MatVectorBodyForce/index.md).

## Example Input File Syntax

!listing modules/tensor_mechanics/test/tests/gravity/mat_vector_body_force.i block=Modules/TensorMechanics/MatVectorBodyForce/all

!syntax parameters /Kernels/MatVectorBodyForce

!syntax inputs /Kernels/MatVectorBodyForce

!syntax children /Kernels/MatVectorBodyForce
