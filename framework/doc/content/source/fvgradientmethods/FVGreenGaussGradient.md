# FVGreenGaussGradient

!syntax description /FVGradientMethods/FVGreenGaussGradient

## Overview

`FVGreenGaussGradient` computes gradients for cell-centered linear finite-volume variables using the
Green-Gauss theorem. A gradient describes how quickly a field changes in space. For a cell $C$ with
volume $V_C$, the method approximates the gradient of a cell-centered field $\phi$ as

!equation
\nabla \phi_C \approx \frac{1}{V_C}\sum_f \phi_f \mathbf{S}_f,

where $f$ ranges over the cell faces, $\phi_f$ is the face value reconstructed from adjacent
cell-centered values and boundary data, and $\mathbf{S}_f$ is the outward face area vector.

Use this method when a linear FV variable or a gradient-based interpolation method needs gradients.
MOOSE also provides two built-in method names for convenience:

- `green-gauss`, equivalent to an `FVGreenGaussGradient` with `limiter = none`
- `green-gauss-venkatakrishnan`, equivalent to an `FVGreenGaussGradient` with
  `limiter = venkatakrishnan`

Named methods in `[FVGradientMethods]` are useful when several variables or interpolation methods
should use the same gradient settings.

## Limiters

The optional [!param](/FVGradientMethods/FVGreenGaussGradient/limiter) parameter controls whether
MOOSE limits the Green-Gauss gradient before using it. With `limiter = none`, the Green-Gauss result
is used directly. With `limiter = venkatakrishnan`, MOOSE first computes the Green-Gauss gradient
and then applies the Venkatakrishnan limiter. A limiter can reduce overshoots near steep solution
changes.

## Example Syntax

Declare a named Green-Gauss gradient method in `[FVGradientMethods]`:

!listing test/tests/variables/linearfv/shared-gradient-method.i block=gg

Use the method as the default gradient method for a linear FV variable through
[!param](/Variables/MooseLinearVariableFVReal/gradient_method):

!listing test/tests/variables/linearfv/shared-gradient-method.i block=u

The same named method can also be used by a gradient-based interpolation method, such as
[FVAdvectedMUSCLDeferredCorrection.md]:

!listing test/tests/variables/linearfv/shared-gradient-method.i block=muscl

!syntax parameters /FVGradientMethods/FVGreenGaussGradient

!syntax inputs /FVGradientMethods/FVGreenGaussGradient

!syntax children /FVGradientMethods/FVGreenGaussGradient
