# MFEMHypreDiagScale

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreDiagScale` solver, which scales a vector by the inverse of the
diagonal of the MFEM equation system matrix. `MFEMHypreDiagScale` is used as a diagonal (Jacobi)
preconditioner. Unlike [MFEMOperatorJacobiSmoother](MFEMOperatorJacobiSmoother.md), it is a hypre
solver, so it can precondition the hypre Krylov solvers [MFEMHyprePCG](MFEMHyprePCG.md),
[MFEMHypreGMRES](MFEMHypreGMRES.md) and [MFEMHypreFGMRES](MFEMHypreFGMRES.md), which only accept
hypre preconditioners. It can also precondition the MFEM native Krylov solvers, such as
[MFEMCGSolver](MFEMCGSolver.md).

The equation system matrix must be assembled, so `MFEMHypreDiagScale` cannot be used with partial
assembly.

!syntax parameters /Solvers/MFEMHypreDiagScale

!syntax inputs /Solvers/MFEMHypreDiagScale

!syntax children /Solvers/MFEMHypreDiagScale

!if-end!

!else
!include mfem/mfem_warning.md
