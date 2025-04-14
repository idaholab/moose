# MFEMOperatorJacobiSmoother

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMOperatorJacobiSmoother

## Overview

Defines and builds an `mfem::OperatorJacobiSmoother` solver to use to perform Jacobi iterations on
the MFEM equation system. Most often used as a preconditioner, compatible with partial assembly.

!syntax parameters /Solver/MFEMOperatorJacobiSmoother

!syntax inputs /Solver/MFEMOperatorJacobiSmoother

!syntax children /Solver/MFEMOperatorJacobiSmoother

!else
!include mfem/mfem_warning.md
