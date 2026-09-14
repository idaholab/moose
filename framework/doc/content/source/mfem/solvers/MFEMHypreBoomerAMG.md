# MFEMHypreBoomerAMG

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreBoomerAMG` solver to use as a preconditioner or solver to solve the MFEM equation system.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMHypreBoomerAMG/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

## Vector unknowns

By default BoomerAMG coarsens every degree of freedom as if it belonged to one scalar unknown.
For a vector unknown, such as a displacement,
[!param](/Solvers/MFEMHypreBoomerAMG/vector_treatment) selects one of hypre's approaches for systems
of PDEs instead:

- `by_component` coarsens each component on its own and does not interpolate between components.
  This is what hypre calls the unknown approach. It corresponds to
  `mfem::HypreBoomerAMG::SetSystemsOptions`, and to MFEM example 2 run without `-elast`.
- `rigid_body_modes` also adds the rigid body modes of the space to the interpolation, which suits
  any vector problem whose near null space is rigid body motion. This is hypre's GM/LN approach. It
  corresponds to `mfem::HypreBoomerAMG::SetElasticityOptions`, and to MFEM example 2 run with
  `-elast`. It is not applied when hypre runs on a GPU.

Both need [!param](/Solvers/MFEMHypreBoomerAMG/fespace), the space of the unknown, which must use
`VDIM` ordering, and both default the strength threshold to 0.5. Neither is always better:
`rigid_body_modes` can take fewer iterations but costs more to set up, and on MFEM example 2
`by_component` is the faster of the two.

For compatibility, setting `fespace` without `vector_treatment` selects `rigid_body_modes`.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Solvers

!syntax parameters /Solvers/MFEMHypreBoomerAMG

!syntax inputs /Solvers/MFEMHypreBoomerAMG

!syntax children /Solvers/MFEMHypreBoomerAMG

!if-end!

!else
!include mfem/mfem_warning.md
