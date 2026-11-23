# MFEMHypreSmoother

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::HypreSmoother` solver, which applies one sweep of a hypre relaxation
method, such as Jacobi, l1-scaled Jacobi or l1-scaled Gauss-Seidel, to the MFEM equation system.
`MFEMHypreSmoother` is used as a preconditioner. The relaxation method is set by
[!param](/Solvers/MFEMHypreSmoother/smoother_type), and defaults to l1-scaled Gauss-Seidel (`l1GS`) when
hypre runs on the CPU and to l1-scaled Jacobi (`l1Jacobi`) when it runs on a GPU.

`MFEMHypreSmoother` is not a hypre solver, so it cannot precondition the hypre Krylov solvers
[MFEMHyprePCG](MFEMHyprePCG.md), [MFEMHypreGMRES](MFEMHypreGMRES.md) and
[MFEMHypreFGMRES](MFEMHypreFGMRES.md). It preconditions the MFEM native Krylov solvers, such as
[MFEMCGSolver](MFEMCGSolver.md), [MFEMGMRESSolver](MFEMGMRESSolver.md) and
[MFEMMINRESSolver](MFEMMINRESSolver.md). For a diagonal preconditioner for the hypre Krylov
solvers, use [MFEMHypreDiagScale](MFEMHypreDiagScale.md).

A Low-Order-Refined (LOR) version of this smoother may be used instead by setting the parameter
[!param](/Solvers/MFEMHypreSmoother/low_order_refined) to `true`. It smooths with the matrix of a
low-order discretization on a refined mesh, which is assembled even when the equation system is
not. Without LOR, the equation system matrix must be assembled, so `MFEMHypreSmoother` cannot be
used with partial assembly.

!syntax parameters /Solvers/MFEMHypreSmoother

!syntax inputs /Solvers/MFEMHypreSmoother

!syntax children /Solvers/MFEMHypreSmoother

!if-end!

!else
!include mfem/mfem_warning.md
