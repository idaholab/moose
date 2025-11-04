# MFEMOperatorJacobiSmoother

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMOperatorJacobiSmoother

## Overview

Defines and builds an `mfem::OperatorJacobiSmoother` solver to use to perform Jacobi iterations on
the MFEM equation system. Most often used as a preconditioner, compatible with partial assembly.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter 
[!param](/Solver/MFEMOperatorJacobiSmoother/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial 
order systems.

!syntax parameters /Solver/MFEMOperatorJacobiSmoother

!syntax inputs /Solver/MFEMOperatorJacobiSmoother

!syntax children /Solver/MFEMOperatorJacobiSmoother

!if-end!

!else
!include mfem/mfem_warning.md
