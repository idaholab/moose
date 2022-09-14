# FVDiffusion

!syntax description /FVKernels/FVDiffusion

The steady-state diffusion equation on a domain $\Omega$ is defined as

!equation
-\nabla \cdot D \nabla u = 0 \in \Omega.

with $D$ the diffusion coefficient or diffusivity. $D$ has to be supplied as material property
to this kernel.

The diffusion term is integrated using the divergence theorem, turning it from a volumetric second
order derivative term into a first order derivative integrated over a surface.

!equation
\int_{element} -\nabla \cdot D \nabla u = \sum_{elem faces f} -D_f \nabla u_f \cdot \vec{n_f} area_f

where $\vec{n_f}$ is the surface normal on each side of the element considered.

The diffusion coefficient can be interpolated to the surface using two approaches:

- +Simple arithmetic average:+ $D_f = w_1 D_1 + (1-w_1) D_2$ (with $D_1$, $D_2$ being the diffusion
  coefficient in the neighboring cells respectively)
- +Simple harmonic average:+ $D_f = \frac{1}{\frac{w_1}{D_1} + \frac{1 - w_1}{D_2}}$, which yields better results
  if the diffusion coefficients are positive and discontinuous. This is due to the fact that this scheme preserves
  flux continuity in the face-normal direction on orthogonal grids.

The interpolation method can be set using the [!param](/FVKernels/FVDiffusion/coeff_interp_method) parameter,
and is defaulted to `harmonic` due to its superior accuracy for discontinuous diffusion coefficients.
Simple tests cases with discontinuous diffusion coefficients (see below)
indicate that using harmonic interpolation yields a second-order accurate
scheme for orthogonal and 1D meshes and close to second-order accurate scheme for slightly
non-orthogonal meshes. At the same time, using a simple arithmetic average for the interpolation of
discontinuous diffusion coefficients yields a first order scheme.

!alert note
This kernel leverages the automatic differentiation system, so the Jacobian is
computed at the same time as the residual and need not be defined separately.

## Example input syntax

This example shows a simple 1D diffusion problem with two variables defined on two subdomains.
Because of the limits of the legacy material system, the two material properties have to have different
names, otherwise it is not clear what the boundary value of the diffusion coefficient should be.

!listing test/tests/fvkernels/block-restriction/1d.i

!syntax parameters /FVKernels/FVDiffusion

!syntax inputs /FVKernels/FVDiffusion

!syntax children /FVKernels/FVDiffusion
