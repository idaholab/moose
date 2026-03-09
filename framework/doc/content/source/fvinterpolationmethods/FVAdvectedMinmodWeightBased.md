# FVAdvectedMinmodWeightBased

## Overview

!include fvinterpolationmethods/limiter_base_overview.md

For minmod, the limiter function is

!equation
\beta(r_f)=\max\!\left(0,\,\min(1, r_f)\right).

Minmod is generally more diffusive than van Leer (it reduces high-order blending more aggressively),
but it is often more robust near steep gradients. For limiter theory and available limiter
definitions in MOOSE, see [Limiters](syntax/Limiters/index.md).

## Example Syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i start=[nvd_minmod] end=[] include-end=true

Use it in a linear FV advection kernel via
[!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name):

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i start=[advection] end=[] include-end=true

!syntax parameters /FVInterpolationMethods/FVAdvectedMinmodWeightBased

!syntax inputs /FVInterpolationMethods/FVAdvectedMinmodWeightBased

!syntax children /FVInterpolationMethods/FVAdvectedMinmodWeightBased
