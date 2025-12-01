# MFEMMUMPS

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMMUMPS

## Overview

Defines and builds an `mfem::MUMPSSolver` to use as a direct solver to solve the MFEM equation system.

!syntax parameters /Solver/MFEMMUMPS

!syntax inputs /Solver/MFEMMUMPS

!syntax children /Solver/MFEMMUMPS

!if-end!

!else
!include mfem/mfem_warning.md
