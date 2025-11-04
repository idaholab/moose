# MFEMHypreFGMRES

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHypreFGMRES

## Overview

Defines and builds an `mfem::HypreFGMRES` solver to use as an iterative solver to solve the MFEM equation system.

Not compatible with use on partially assembled equation systems.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter 
[!param](/Solver/MFEMHypreFGMRES/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial 
order systems.

!syntax parameters /Solver/MFEMHypreFGMRES

!syntax inputs /Solver/MFEMHypreFGMRES

!syntax children /Solver/MFEMHypreFGMRES

!if-end!

!else
!include mfem/mfem_warning.md
