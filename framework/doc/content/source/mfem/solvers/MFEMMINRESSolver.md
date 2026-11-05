# MFEMMINRESSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::MINRESSolver` solver to use as an iterative solver to solve the MFEM
equation system. Compatible with use on partially assembled equation systems.

MINRES requires a symmetric system matrix, which may be indefinite, and a symmetric positive
definite preconditioner. This makes it suitable for saddle-point systems such as mixed Darcy
problems preconditioned with [MFEMBlockDiagonalPreconditioner.md]. Unlike GMRES, it stores a
fixed number of vectors and its cost per iteration does not grow, so it never restarts.

Convergence is measured in the norm of the residual weighted by the inverse of the
preconditioner, so [!param](/Solvers/MFEMMINRESSolver/l_tol) is not directly comparable to the
same tolerance on [MFEMGMRESSolver.md].

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMMINRESSolver/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

!syntax parameters /Solvers/MFEMMINRESSolver

!syntax inputs /Solvers/MFEMMINRESSolver

!syntax children /Solvers/MFEMMINRESSolver

!if-end!

!else
!include mfem/mfem_warning.md
