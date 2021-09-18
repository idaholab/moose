# DGDiffusion

!syntax description /DGKernels/DGDiffusion

More information about the discontinuous Galerkin method may be found in the
[DGKernels syntax page](/syntax/DGKernels/index.md).

# Example input syntax

This example is a 2D diffusion-reaction-source case using DG. It's not clear why there
are both a DGKernel and a Kernel for diffusion.

!listing test/tests/dgkernels/2d_diffusion_dg/dg_stateful.i block=DGKernels

!syntax parameters /DGKernels/DGDiffusion

!syntax inputs /DGKernels/DGDiffusion

!syntax children /DGKernels/DGDiffusion
