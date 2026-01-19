# FVAdvectedVenkatakrishnanDeferredCorrection

## Overview

This object provides a multi-dimensional MUSCL-type reconstruction for
advected quantities that uses a Venkatakrishnan limiter on cell gradients and applies the
high-order component through deferred correction. This yields second-order accuracy in smooth
regions while controlling oscillations near steep gradients and improving robustness of fully
implicit linear FV solves by keeping the matrix contribution low-order ([!cite](venkatakrishnan1993),
[!cite](jasak1996error), [!cite](moukalled2016finite)).

Let $\phi_U$ denote the upwind cell-centered value on a face and let $\nabla \phi_U^{\,L}$ be a
limited gradient (Venkatakrishnan-limited). A MUSCL reconstruction forms a higher-order face value
from the upwind cell:

!equation
\phi_f^{HO} = \phi_U + \nabla \phi_U^{\,L}\cdot(\mathbf{x}_f - \mathbf{x}_U),

where $\mathbf{x}_U$ is the upwind cell centroid and $\mathbf{x}_f$ is a face
location projected onto the centroid-to-centroid line.

Deferred correction splits the face value into a low-order implicit part and a high-order explicit
correction:

!equation
\phi_f = \phi_f^{LO} + \gamma\,(\phi_f^{HO} - \phi_f^{LO}),

where $\phi_f^{LO}=\phi_U$ is the upwind (first-order) face value and $\gamma$ is controlled by
[!param](/FVInterpolationMethods/FVAdvectedVenkatakrishnanDeferredCorrection/deferred_correction_factor).
With $\gamma=0$ the method reduces to pure upwind; with $\gamma=1$ it applies the full MUSCL
reconstruction while still assembling the matrix with the upwind weights and placing the correction
explicitly on the right-hand side. Values $0<\gamma<1$ are sometimes useful for fixed-point
robustness.

For limiter definitions and behavior, see [Limiters](syntax/Limiters/index.md).

## Example Syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=muscl_venkat

Use it in a linear FV advection kernel via
[!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name):

!listing test/tests/linearfvkernels/advection/diagonal-step-2d.i block=advection replace=['advected_interp_method_name = nvd_minmod','advected_interp_method_name = muscl_venkat']

!syntax parameters /FVInterpolationMethods/FVAdvectedVenkatakrishnanDeferredCorrection

!syntax inputs /FVInterpolationMethods/FVAdvectedVenkatakrishnanDeferredCorrection

!syntax children /FVInterpolationMethods/FVAdvectedVenkatakrishnanDeferredCorrection
