# MFEMHyprePCG

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHyprePCG

## Overview

Defines and builds an `mfem::HyprePCG` solver to use as an iterative solver to solve the MFEM equation system.

Not compatible with use on partially assembled equation systems.

!syntax parameters /Solver/MFEMHyprePCG

!syntax inputs /Solver/MFEMHyprePCG

!syntax children /Solver/MFEMHyprePCG

!else
!include mfem/mfem_warning.md
