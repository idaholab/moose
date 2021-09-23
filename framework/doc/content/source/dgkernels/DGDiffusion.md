# DGDiffusion

!syntax description /DGKernels/DGDiffusion

More information about the discontinuous Galerkin method, and in particular for the Poisson equation,
may be found in the [DGKernels syntax page](syntax/DGKernels/index.md).

## Example input syntax

This example is a 2D diffusion-reaction-source case using DG. The kernels are taking care of the
volumetric terms in the equation, while the `DGDiffusion` DGKernel is defined on the element sides.

!listing test/tests/dgkernels/2d_diffusion_dg/dg_stateful.i block=Kernels DGKernels

!syntax parameters /DGKernels/DGDiffusion

!syntax inputs /DGKernels/DGDiffusion

!syntax children /DGKernels/DGDiffusion
