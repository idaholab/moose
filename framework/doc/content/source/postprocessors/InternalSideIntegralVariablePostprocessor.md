# SideIntegralVariablePostprocessor

!syntax description /Postprocessors/InternalSideIntegralVariablePostprocessor

The InternalSideIntegralVariablePostprocessor is also an intermediate
base class that should be derived from for any calculation involving
the integral of a variable quantity over internal sides.

## Example input syntax

In this example, an `InternalSideIntegralVariablePostprocessor` is
used to compute the integral of variable `u` over all internal sides.

!listing test/tests/postprocessors/internal_side_integral/internal_side_integral_test.i block=Postprocessors

!syntax parameters /Postprocessors/InternalSideIntegralVariablePostprocessor

!syntax inputs /Postprocessors/InternalSideIntegralVariablePostprocessor

!syntax children /Postprocessors/InternalSideIntegralVariablePostprocessor
