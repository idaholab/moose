# MFEMHypreBoomerAMG

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreBoomerAMG` solver to use as a preconditioner or solver to solve the MFEM equation system.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMHypreBoomerAMG/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Preconditioner Solvers

!syntax parameters /Solvers/MFEMHypreBoomerAMG

!syntax inputs /Solvers/MFEMHypreBoomerAMG

!syntax children /Solvers/MFEMHypreBoomerAMG

!if-end!

!else
!include mfem/mfem_warning.md
