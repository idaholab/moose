# FVAdvectedMUSCLDeferredCorrection

## Overview

This interpolation method provides a multi-dimensional MUSCL-type reconstruction for advected
quantities and applies the high-order component through deferred correction. The cell gradients used
for the reconstruction are selected with
[!param](/FVInterpolationMethods/FVAdvectedMUSCLDeferredCorrection/gradient_method). This yields
second-order accuracy in smooth regions while improving robustness of fully implicit linear FV
solves by keeping the matrix contribution low-order ([!cite](venkatakrishnan1993),
[!cite](jasak1996error), [!cite](moukalled2016finite)).

Let $\phi_U$ denote the upwind cell-centered value on a face and let $\nabla \phi_U$ be the selected
cell gradient. A MUSCL reconstruction forms a higher-order face value from the upwind cell:

!equation
\phi_f^{HO} = \phi_U + \nabla \phi_U\cdot(\mathbf{x}_f - \mathbf{x}_U),

where $\mathbf{x}_U$ is the upwind cell centroid and $\mathbf{x}_f$ is a face
centroid. In MOOSE, the MUSCL reconstruction uses this face point, so on skewed meshes the skewness
is included directly in the reconstruction rather than added afterward as a separate correction.

Deferred correction splits the face value into a low-order implicit part and a high-order explicit
correction:

!equation
\phi_f = \phi_f^{LO} + \gamma\,(\phi_f^{HO} - \phi_f^{LO}),

where $\phi_f^{LO}=\phi_U$ is the upwind (first-order) face value and $\gamma$ is controlled by
[!param](/FVInterpolationMethods/FVAdvectedMUSCLDeferredCorrection/deferred_correction_factor).
With $\gamma=0$ the method reduces to pure upwind; with $\gamma=1$ it applies the full MUSCL
reconstruction while still assembling the matrix with the upwind weights and placing the correction
explicitly on the right-hand side. Values $0<\gamma<1$ are sometimes useful for fixed-point iteration
robustness.

## Gradient method and boundedness

The default gradient method is `green-gauss-venkatakrishnan`, which uses a Green-Gauss gradient with
a Venkatakrishnan limiter. A different named method can be selected with
[!param](/FVInterpolationMethods/FVAdvectedMUSCLDeferredCorrection/gradient_method).

This method does *not* guarantee strict boundedness. The default Venkatakrishnan limiter is smooth
and intentionally less restrictive than a hard clipping procedure in order to avoid degrading smooth
extrema. As a result, small overshoots or undershoots may still occur. In addition, this method
applies the high-order term through deferred correction on the right-hand side, so the overall
discretization is not a strict monotone or maximum-principle-preserving scheme.

The positive effect, however, is that this advection discretization is second order on skewed meshes
as well.

For limiter definitions and behavior, see [Limiters](syntax/Limiters/index.md).

## Example Syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=muscl

Use it in a linear FV advection kernel via
[!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name):

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=advection replace=['advected_interp_method_name = nvd_minmod','advected_interp_method_name = muscl']

!syntax parameters /FVInterpolationMethods/FVAdvectedMUSCLDeferredCorrection

!syntax inputs /FVInterpolationMethods/FVAdvectedMUSCLDeferredCorrection

!syntax children /FVInterpolationMethods/FVAdvectedMUSCLDeferredCorrection
