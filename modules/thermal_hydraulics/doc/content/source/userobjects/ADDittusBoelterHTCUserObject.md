# ADDittusBoelterHTCUserObject

## Overview

This user object computes the wall heat transfer coefficient $h$ of a lumped flow segment
of the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md)
from the Dittus-Boelter correlation, [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#dittus_boelter),
and supplies it as a [functor](/Functors/index.md) named after the object itself. The
conjugate heat transfer scalar kernels, [ADPipeInnerWallTemperatureScalarKernel.md] and
[ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel.md], consume it through their
`heat_transfer_coefficient` parameter, which lets a single definition of the correlation
serve both the primary and secondary sides of a segment.

The Reynolds number is formed from the segment's mass flux, $G = \dot{m} / A$, so no
density evaluation is needed. Fluid properties are evaluated at the segment's bulk film
temperature, the mean of the segment fluid temperature $T_f$ and its upwinded inlet value
$T_{in}$, taken from the nominal inlet or outlet neighbor according to the sign of
$\dot{m}$.

## Why this is a user object rather than a functor material

The consuming kernels act on `SCALAR` family variables. A scalar kernel has no current
element, because SCALAR degrees of freedom are global and carry no mesh location, so it can
only evaluate a functor with a null-element argument. A functor material property is
block-piecewise and must dereference that element to select the lambda for its subdomain,
which makes it unusable here. This object therefore supplies $h$ as an
element-independent functor: the value depends only on scalar variables and on the
segment's geometry, so every spatial argument yields the same result.

For the same reason, the `reference_pressure`, `flow_area`, and `wetted_perimeter`
parameters must be given as plain numbers, postprocessors, or scalar variables, all of
which are element-independent. A [Function](syntax/Functions/index.md) name is not valid
here, since evaluating a function requires a spatial point that this object cannot supply.

!syntax parameters /UserObjects/ADDittusBoelterHTCUserObject

!syntax inputs /UserObjects/ADDittusBoelterHTCUserObject

!syntax children /UserObjects/ADDittusBoelterHTCUserObject
