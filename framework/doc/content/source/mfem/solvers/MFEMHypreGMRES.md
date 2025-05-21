# MFEMHypreGMRES

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHypreGMRES

## Overview

Defines and builds an `mfem::HypreGMRES` solver to use as an iterative solver to solve the MFEM
equation system.

Not compatible with use on partially assembled equation systems.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter 
[!param](/Solvers/MFEMHypreGMRES/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial 
order systems.

!syntax parameters /Solver/MFEMHypreGMRES

!syntax inputs /Solver/MFEMHypreGMRES

!syntax children /Solver/MFEMHypreGMRES

!if-end!

!else
!include mfem/mfem_warning.md
