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
centroid. In MOOSE, both the Venkatakrishnan limiting step for the cell gradient and the MUSCL
reconstruction use this same face point, so on skewed meshes the skewness is included directly in
the limited reconstruction rather than added afterward as a separate correction.

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

## Notes on boundedness and skewness

Using the actual face centroid in both the limiter and the MUSCL reconstruction makes the scheme
internally consistent on skewed meshes: the limited gradient is constrained against the same face
location at which the higher-order value is reconstructed.

This does *not* guarantee strict boundedness of the final scheme. The Venkatakrishnan
limiter is a smooth limiter with a small relaxation term, which is intentionally less restrictive
than a hard clipping procedure in order to avoid degrading smooth extrema. As a result, small
overshoots or undershoots may still occur. In addition, this object applies the high-order term
through deferred correction on the right-hand side, so the overall discretization is not a strict
monotone or maximum-principle-preserving scheme.

The positive effect, however, is that this advection discretization second order on skewed meshes as well.

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
