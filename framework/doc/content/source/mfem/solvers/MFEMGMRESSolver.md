# MFEMGMRESSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::GMRESSolver` solver to use as an iterative solver to solve the MFEM
equation system. Compatible with use on partially assembled equation systems.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter
[!param](/Solvers/MFEMGMRESSolver/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial
order systems.

!syntax parameters /Solvers/MFEMGMRESSolver

!syntax inputs /Solvers/MFEMGMRESSolver

!syntax children /Solvers/MFEMGMRESSolver

!if-end!

!else
!include mfem/mfem_warning.md
