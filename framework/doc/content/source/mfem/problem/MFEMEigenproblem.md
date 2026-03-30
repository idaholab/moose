# MFEMEigenproblem

!if! function=hasCapability('mfem')

## Overview

Specialization of [MFEMProblem](problem/MFEMProblem.md) for building and solving a finite element
generalized eigenvalue problem using the MFEM finite element library. The eigenproblem takes the
form: find $(\lambda, u)$ such that

$$A u = \lambda M u,$$

where $A$ is the stiffness matrix assembled from the problem kernels and $M$ is a mass matrix
constructed by the eigensolver using the provided scalar coefficient.

For each variable declared in the `[Variables]` block, `MFEMEigenproblem` automatically creates
`num_modes` additional grid functions (named `<variable>_0`, `<variable>_1`, etc.) to store the
computed eigenvectors. The solver must be an eigensolver derived from `MFEMEigensolverBase`, such
as [MFEMHypreLOBPCG](solvers/MFEMHypreLOBPCG.md) or [MFEMHypreAME](solvers/MFEMHypreAME.md);
specifying a non-eigensolver will result in an error.

Computed eigenvalues can be exported using
[MFEMEigenvaluesPostprocessor](vectorpostprocessors/MFEMEigenvaluesPostprocessor.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_eigenproblem.i block=Problem Variables Solver

!syntax parameters /Problem/MFEMEigenproblem

!syntax inputs /Problem/MFEMEigenproblem

!syntax children /Problem/MFEMEigenproblem

!if-end!

!else
!include mfem/mfem_warning.md
