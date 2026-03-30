# MFEMHypreLOBPCG

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreLOBPCG` eigensolver to compute the lowest eigenmodes of a
generalized eigenvalue problem using the MFEM finite element library. LOBPCG (Locally Optimal
Block Preconditioned Conjugate Gradient) is an iterative eigensolver well suited to large-scale
problems arising from finite element discretizations.

`MFEMHypreLOBPCG` requires a scalar coefficient to construct the mass matrix, specified via the
[!param](/Solver/MFEMHypreLOBPCG/coefficient) parameter.

The number of eigenmodes to compute is controlled by the `num_modes` parameter on the
[MFEMEigenproblem](problem/MFEMEigenproblem.md) problem type. This solver cannot be used in
conjunction with Low-Order-Refined (LOR) preconditioning.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_eigenproblem.i block=Preconditioner Solver

!syntax parameters /Solver/MFEMHypreLOBPCG

!syntax inputs /Solver/MFEMHypreLOBPCG

!syntax children /Solver/MFEMHypreLOBPCG

!if-end!

!else
!include mfem/mfem_warning.md
