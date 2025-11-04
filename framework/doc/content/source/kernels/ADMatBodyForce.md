# ADMatBodyForce

## Description

`MatBodyForce` implements a force term given via a material property. This kernel leverages
the formation in [ADBodyForce](ADBodyForce.md) by multiplying the residual by a material property.
All other implementation details are the same as [ADBodyForce](ADBodyForce.md).

## Example Syntax

!listing test/tests/kernels/body_force/ad_mat_forcing_function_test.i block=forcing

!syntax parameters /Kernels/ADMatBodyForce

!syntax inputs /Kernels/ADMatBodyForce

!syntax children /Kernels/ADMatBodyForce
