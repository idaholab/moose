# MFEMHypreAME

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreAME` eigensolver to compute the lowest eigenmodes of a
generalized eigenvalue problem using the MFEM finite element library. AME (Auxiliary-space Maxwell
Eigensolver) is designed specifically for Maxwell eigenproblems discretized with Nédélec (edge)
elements in $H(\mathrm{curl})$ spaces, and should be used in conjunction with an
[MFEMHypreAMS](solvers/MFEMHypreAMS.md) preconditioner.

`MFEMHypreAME` requires a scalar coefficient to construct the mass matrix, specified via the
[!param](/Solver/MFEMHypreAME/coefficient) parameter. When solving a Maxwell eigenproblem, the
`MFEMHypreAMS` preconditioner should be constructed with `singular = true` to account for the
kernel of the curl-curl operator.

The number of eigenmodes to compute is controlled by the `num_modes` parameter on the
[MFEMEigenproblem](problem/MFEMEigenproblem.md) problem type. This solver cannot be used in
conjunction with Low-Order-Refined (LOR) preconditioning.

## Example Input File Syntax

!listing test/tests/mfem/kernels/maxwell_eigenproblem.i block=Preconditioner Solver

!syntax parameters /Solver/MFEMHypreAME

!syntax inputs /Solver/MFEMHypreAME

!syntax children /Solver/MFEMHypreAME

!if-end!

!else
!include mfem/mfem_warning.md
