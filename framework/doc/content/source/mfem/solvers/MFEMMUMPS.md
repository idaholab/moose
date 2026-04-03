# MFEMMUMPS

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::MUMPSSolver` to use as a direct solver to solve the MFEM equation system.

!syntax parameters /Solvers/MFEMMUMPS

!syntax inputs /Solvers/MFEMMUMPS

!syntax children /Solvers/MFEMMUMPS

!if-end!

!else
!include mfem/mfem_warning.md
