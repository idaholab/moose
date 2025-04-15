# MFEMHypreGMRES

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMHypreGMRES

## Overview

Defines and builds an `mfem::HypreGMRES` solver to use as an iterative solver to solve the MFEM
equation system.

Not compatible with use on partially assembled equation systems.

!syntax parameters /Solver/MFEMHypreGMRES

!syntax inputs /Solver/MFEMHypreGMRES

!syntax children /Solver/MFEMHypreGMRES

!if-end!

!else
!include mfem/mfem_warning.md
