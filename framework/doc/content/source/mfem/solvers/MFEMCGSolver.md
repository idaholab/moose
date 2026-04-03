# MFEMCGSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::CGSolver` solver to use as an iterative solver to solve the MFEM
equation system. Compatible with use on partially assembled equation systems.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMCGSolver/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

!syntax parameters /Solvers/MFEMCGSolver

!syntax inputs /Solvers/MFEMCGSolver

!syntax children /Solvers/MFEMCGSolver

!if-end!

!else
!include mfem/mfem_warning.md
