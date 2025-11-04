# MatBodyForce

## Description

`MatBodyForce` implements a force term given via a material property. This kernel leverages
the formation in [BodyForce](BodyForce.md) by multiplying the jacobian and residual by a
material property. Jacobian terms are given by leveraging the
[DerivativeMaterialInterface](/DerivativeMaterialInterface.md). All other implementation
details are the same as [BodyForce](BodyForce.md).

## Example Syntax

!listing test/tests/kernels/body_force/mat_forcing_function_test.i block=forcing

!syntax parameters /Kernels/MatBodyForce

!syntax inputs /Kernels/MatBodyForce

!syntax children /Kernels/MatBodyForce
