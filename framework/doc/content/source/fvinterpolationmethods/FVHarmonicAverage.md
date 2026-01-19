# FVHarmonicAverage

## Overview

This object computes a weighted harmonic mean of two cell-centered values when
interpolating to an internal face. Harmonic averaging is commonly used for face diffusion
coefficients because it provides the appropriate effective property when two adjacent cells act
like resistances in series, improving flux continuity across strong coefficient jumps
([!cite](moukalled2016finite), [!cite](jasak1996error)).

Let $\phi_C$ and $\phi_N$ be cell-centered values on the element and neighbor sides of a face,
and let $g_C\in[0,1]$ be the geometric weight associated with the element side. The weighted
harmonic mean is

!equation
\phi_f = \left(\frac{g_C}{\phi_C} + \frac{1-g_C}{\phi_N}\right)^{-1}.

In typical applications $\phi$ is positive (e.g., conductivity or diffusivity). When $\phi$ can
change sign, harmonic averaging is not generally appropriate; a consistent sign is needed for the
mean to behave as intended.

## Example Syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i block=harm

Use it for a coefficient functor via
[!param](/LinearFVKernels/LinearFVDiffusion/coeff_interp_method):

!listing test/tests/linearfvkernels/diffusion/diffusion-1d.i block=diffusion replace=['diffusion_coeff = coeff_func','diffusion_coeff = coeff_pos_func\n    coeff_interp_method = harm']

## Useful Links

See [FVInterpolationMethod.md] for the base class and [LinearFVDiffusion.md] for
typical usage.

!syntax parameters /FVInterpolationMethods/FVHarmonicAverage

!syntax inputs /FVInterpolationMethods/FVHarmonicAverage

!syntax children /FVInterpolationMethods/FVHarmonicAverage
