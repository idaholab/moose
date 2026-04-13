# FVAdvectedVanLeerWeightBased

## Overview

!include fvinterpolationmethods/limiter_base_overview.md

For van Leer, the limiter function is

!equation
\beta(r_f)=\frac{r_f + |r_f|}{1 + |r_f|}.

For $r_f\le 0$, $\beta=0$ and the method reverts to upwind near extrema/discontinuities,
preventing spurious oscillations. For $r_f>0$, van Leer provides a smooth transition (less
diffusive than minmod in many regions), which can better preserve sharp but smooth features.

For a more diffusive (often more robust) alternative, see [FVAdvectedMinmodWeightBased.md]. For
limiter theory and available limiter definitions in MOOSE, see [Limiters](syntax/Limiters/index.md).

## Example Syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=nvd_vanleer

Use it in a linear FV advection kernel via
[!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name):

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=advection replace=['advected_interp_method_name = nvd_minmod','advected_interp_method_name = nvd_vanleer']

!syntax parameters /FVInterpolationMethods/FVAdvectedVanLeerWeightBased

!syntax inputs /FVInterpolationMethods/FVAdvectedVanLeerWeightBased

!syntax children /FVInterpolationMethods/FVAdvectedVanLeerWeightBased
