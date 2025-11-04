# MFEMSuperLU

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMSuperLU

## Overview

Defines and builds an `mfem::SuperLUSolver` to use as a direct solver to solve the MFEM equation system.

!syntax parameters /Solver/MFEMSuperLU

!syntax inputs /Solver/MFEMSuperLU

!syntax children /Solver/MFEMSuperLU

!if-end!

!else
!include mfem/mfem_warning.md
