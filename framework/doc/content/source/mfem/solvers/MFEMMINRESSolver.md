# MFEMMINRESSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::MINRESSolver` solver to use as an iterative solver to solve the MFEM
equation system with the minimal residual method. `MFEMMINRESSolver` requires the system to be
symmetric, but not positive definite, so it applies to symmetric indefinite systems such as the
saddle-point systems of mixed formulations, which [MFEMCGSolver](MFEMCGSolver.md) cannot solve.
Unlike [MFEMGMRESSolver](MFEMGMRESSolver.md), it needs a fixed amount of memory regardless of the
number of iterations. Compatible with use on partially assembled equation systems.

A preconditioner given by [!param](/Solvers/MFEMMINRESSolver/preconditioner) must be symmetric
positive definite.

!syntax parameters /Solvers/MFEMMINRESSolver

!syntax inputs /Solvers/MFEMMINRESSolver

!syntax children /Solvers/MFEMMINRESSolver

!if-end!

!else
!include mfem/mfem_warning.md
