# FVAdvectedVanLeerWeightBased

## Overview

This object provides a second-order, TVD-style scheme for interpolating a cell-centered
advected quantity to a finite-volume face on unstructured grids. It is implemented as a *limited
blending weight* between upwind and linear (geometric) interpolation, allowing the face value to be
assembled using only two-cell (elem/neighbor) matrix weights (no MUSCL reconstruction, no deferred
correction) ([!cite](moukalled2016finite), [!cite](jasak1996error), [!cite](greenshieldsweller2022), [!cite](harten1997)).

Let \(\phi_U\) and \(\phi_D\) denote the upwind and downwind cell-centered values on a face (as
determined by the sign of the face mass flux). The face value is written as

!equation
\phi_f = (1-g)\,\phi_U + g\,\phi_D,

with

!equation
g = \alpha\,\beta(r_f)\,(1-w_f).

Here \(\alpha\) is the user scaling factor
([!param](/InterpolationMethods/FVAdvectedVanLeerWeightBased/blending_factor)), \(w_f\) is the
geometric *linear-interpolation* weight associated with the upwind cell, and \(\beta(r_f)\) is a
limiter coefficient computed from a smoothness indicator \(r_f\) (typically built from the upwind
cell gradient and the centroid-to-centroid vector on unstructured meshes; see
[!cite](moukalled2016finite)).

For van Leer, the limiter function is

!equation
\beta(r_f)=\frac{r_f + |r_f|}{1 + |r_f|}.

For \(r_f\le 0\), \(\beta=0\) and the method reverts to upwind near extrema/discontinuities,
preventing spurious oscillations. For \(r_f>0\), van Leer provides a smooth transition (less
diffusive than minmod in many regions), which can better preserve sharp but smooth features.

Unlike minmod, van Leer can produce \(\beta(r_f)>1\) when \(r_f>1\), which would make the scheme
*compressive* (more downwind-biased than linear) if used without additional constraints. To avoid
downwind-biased weights that can degrade robustness (especially for fully implicit linear systems),
this method enables [!param](/InterpolationMethods/FVAdvectedVanLeerWeightBased/limit_to_linear) by
default, clamping the blending to

!equation
0 \le g \le 1-w_f,

so the method blends only between upwind and linear interpolation.

For a more diffusive (often more robust) alternative, see [FVAdvectedMinmodWeightBased.md]. For
limiter theory and available limiter definitions in MOOSE, see [Limiters](syntax/Limiters/index.md).

## Example Syntax

Declare the interpolation method in `[InterpolationMethods]`:

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=nvd_vanleer

Use it in an advection kernel via
[!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name):

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=advection

!syntax parameters /InterpolationMethods/FVAdvectedVanLeerWeightBased

!syntax inputs /InterpolationMethods/FVAdvectedVanLeerWeightBased

!syntax children /InterpolationMethods/FVAdvectedVanLeerWeightBased
