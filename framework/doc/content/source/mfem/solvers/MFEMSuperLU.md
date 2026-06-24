# MFEMSuperLU

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::SuperLUSolver` to use as a direct solver to solve the MFEM equation system.

!syntax parameters /Solvers/MFEMSuperLU

!syntax inputs /Solvers/MFEMSuperLU

!syntax children /Solvers/MFEMSuperLU

!if-end!

!else
!include mfem/mfem_warning.md
