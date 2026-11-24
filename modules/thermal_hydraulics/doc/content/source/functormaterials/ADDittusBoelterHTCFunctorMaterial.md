# ADDittusBoelterHTCFunctorMaterial

## Overview

This functor material computes the wall heat transfer coefficient $h$ of a lumped flow segment
of the [Path-integrated incompressible flow model](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md)
from the Dittus-Boelter correlation, [!eqref](modules/thermal_hydraulics/theory_manual/path_integrated_incompressible_model/index.md#dittus_boelter),
and declares it as a functor material property. The property is named by the `htc_name`
parameter or, if that is omitted, after the object itself. The conjugate heat transfer scalar
kernels, [ADPipeInnerWallTemperatureScalarKernel.md] and
[ADPipeOuterWallCoupledConvectiveTemperatureScalarKernel.md], consume it through their
`heat_transfer_coefficient` parameter, which lets a single definition of the correlation
serve both the primary and secondary sides of a segment.

The Reynolds number is formed from the segment's mass flux, $G = \dot{m} / A$, so no
density evaluation is needed. Fluid properties are evaluated at the segment's bulk film
temperature, the mean of the segment fluid temperature $T_f$ and its upwinded inlet value
$T_{in}$, taken from the nominal inlet or outlet neighbor according to the sign of
$\dot{m}$.

## Evaluation

Like any functor material property, the coefficient is defined only on the blocks of this
material, and consumers must evaluate it with a spatial argument located on one of those
blocks. The `reference_pressure`, `flow_area`, and `wetted_perimeter` functors are evaluated
at that same spatial argument. The coupled mass flow rate and fluid temperatures are `SCALAR`
variables, so they do not vary in space.

!syntax parameters /FunctorMaterials/ADDittusBoelterHTCFunctorMaterial

!syntax inputs /FunctorMaterials/ADDittusBoelterHTCFunctorMaterial

!syntax children /FunctorMaterials/ADDittusBoelterHTCFunctorMaterial
