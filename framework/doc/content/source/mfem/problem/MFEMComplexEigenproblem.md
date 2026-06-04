# MFEMComplexEigenproblem

!if! function=hasCapability('mfem')

## Overview

Specialization of [MFEMProblem](problem/MFEMProblem.md) for building and solving a complex-valued
finite element generalized eigenvalue problem using the MFEM finite element library. The
eigenproblem takes the form: find $(\lambda, u)$ such that

$$A u = \lambda M u,$$

where $A$ is the Hermitian stiffness matrix assembled from the problem kernels and $M$ is a mass
matrix scaled by the [!param](/Problem/MFEMComplexEigenproblem/rhs_coefficient) coefficient. The
complex system is assembled in its monolithic real form and solved with the real eigensolvers, and
the computed eigenmodes are stored as complex grid functions.

[!param](/Problem/MFEMComplexEigenproblem/numeric_type) must be set to `complex`; specifying
anything else will result in an error. For a real-valued eigenproblem,
use [MFEMEigenproblem](problem/MFEMEigenproblem.md) instead.

As with [MFEMEigenproblem](problem/MFEMEigenproblem.md), for each variable declared in the
`[Variables]` block a further [!param](/Problem/MFEMComplexEigenproblem/num_modes) grid functions
are created (named `<variable><mode_separator><index>`, e.g. `u_0`, `u_1`, etc. with the default
separator `_`) to store the computed eigenvectors. The separator can be changed via
[!param](/Problem/MFEMComplexEigenproblem/mode_separator) when the default would clash with names
already in use (for example, names used for components of vector fields). The solver must be an
eigensolver derived from `MFEMEigensolverBase`, such
as [MFEMHypreLOBPCG](solvers/MFEMHypreLOBPCG.md) or [MFEMHypreAME](solvers/MFEMHypreAME.md);
specifying a non-eigensolver will result in an error.

Computed eigenvalues can be exported using
[MFEMEigenvaluesPostprocessor](vectorpostprocessors/MFEMEigenvaluesPostprocessor.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/complex_diffusion_eigenproblem.i block=Problem Variables Solver

!syntax parameters /Problem/MFEMComplexEigenproblem

!syntax inputs /Problem/MFEMComplexEigenproblem

!syntax children /Problem/MFEMComplexEigenproblem

!if-end!

!else
!include mfem/mfem_warning.md
