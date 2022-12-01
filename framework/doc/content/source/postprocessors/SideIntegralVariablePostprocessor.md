# SideIntegralVariablePostprocessor

!syntax description /Postprocessors/SideIntegralVariablePostprocessor

The SideIntegralVariablePostprocessor is also an intermediate base class
that should be derived from for any calculation involving
the integral of a variable quantity over a side.

## Example input syntax

In this example, a `SideIntegralVariablePostprocessor` is used to compute the integral
of variable `u` over the sideset of id `0`.

!listing test/tests/postprocessors/side_integral/side_integral_test.i block=Postprocessors

!syntax parameters /Postprocessors/SideIntegralVariablePostprocessor

!syntax inputs /Postprocessors/SideIntegralVariablePostprocessor

!syntax children /Postprocessors/SideIntegralVariablePostprocessor
