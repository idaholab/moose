# FVGradientMethods System

`FVGradientMethods` lists the named gradient methods available to linear finite-volume variables and
gradient-based interpolation methods. A gradient describes how quickly a cell-centered field changes
in space.

The default gradient method for [MooseLinearVariableFV.md] is configured with
[!param](/Variables/MooseLinearVariableFVReal/gradient_method). This parameter may name a built-in
method such as `green-gauss` or `green-gauss-venkatakrishnan`, or it may name a method declared in
`[FVGradientMethods]`. Gradient-based interpolation methods can also request a named method directly,
for example
[!param](/FVInterpolationMethods/FVAdvectedVenkatakrishnanDeferredCorrection/gradient_method).

## Using named gradient methods

If several parts of an input file should use the same gradient settings, declare the method once in
`[FVGradientMethods]` and refer to it by name. This avoids repeating the same settings and makes it
clear which variables and interpolation methods use the same gradient choice.

## FVGradientMethods block

Declare methods in the `[FVGradientMethods]` block. The example below defines one named
Green-Gauss method:

!listing test/tests/variables/linearfv/shared-gradient-method.i
         block=FVGradientMethods

The method name can then be referenced by a variable:

!listing test/tests/variables/linearfv/shared-gradient-method.i
         block=u

or by a gradient-based interpolation method:

!listing test/tests/variables/linearfv/shared-gradient-method.i
         block=muscl

See the individual method pages listed below for details.

!syntax list /FVGradientMethods objects=True actions=False subsystems=False
