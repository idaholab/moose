# MFEMHyprePCG

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHyprePCG

## Overview

Defines and builds an `mfem::HyprePCG` solver to use as an iterative solver to solve the MFEM equation system.

Not compatible with use on partially assembled equation systems.

A Low-Order-Refined (LOR) version of this solver may be used instead by setting the parameter 
[!param](/Solvers/MFEMHyprePCG/low_order_refined) to `true`. Using an LOR solver improves performance for high polynomial 
order systems.

!syntax parameters /Solver/MFEMHyprePCG

!syntax inputs /Solver/MFEMHyprePCG

!syntax children /Solver/MFEMHyprePCG

!if-end!

!else
!include mfem/mfem_warning.md
