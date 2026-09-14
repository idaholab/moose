# MFEMHypreBoomerAMG

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreBoomerAMG` solver to use as a preconditioner or solver to solve the MFEM equation system.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMHypreBoomerAMG/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

## Vector unknowns

By default BoomerAMG coarsens every degree of freedom as if it belonged to one scalar unknown.
For a vector unknown, such as a displacement, [!param](/Solvers/MFEMHypreBoomerAMG/system_type)
selects one of hypre's approaches for systems of PDEs instead:

- `systems` coarsens each component on its own and does not interpolate between components. It
  corresponds to `mfem::HypreBoomerAMG::SetSystemsOptions`, and to MFEM example 2 run without
  `-elast`.
- `elasticity` also adds the rigid body modes of the space to the interpolation. It corresponds
  to `mfem::HypreBoomerAMG::SetElasticityOptions`, and to MFEM example 2 run with `-elast`. It is
  not applied when hypre runs on a GPU.

Both need [!param](/Solvers/MFEMHypreBoomerAMG/fespace), the space of the unknown, which must use
`VDIM` ordering, and both default the strength threshold to 0.5. Neither is always better: the
elasticity approach suits problems dominated by rigid body motion, but costs more to set up, and
on MFEM example 2 the `systems` approach is the faster of the two.

For compatibility, setting `fespace` without `system_type` selects `elasticity`.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Solvers

!syntax parameters /Solvers/MFEMHypreBoomerAMG

!syntax inputs /Solvers/MFEMHypreBoomerAMG

!syntax children /Solvers/MFEMHypreBoomerAMG

!if-end!

!else
!include mfem/mfem_warning.md
