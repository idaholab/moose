# MFEMGMRESSolver

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/MFEMGMRESSolver

## Overview

Defines and builds an `mfem::GMRESSolver` solver to use as an iterative solver to solve the MFEM
equation system. Compatible with use on partially assembled equation systems.

!syntax parameters /Solver/MFEMGMRESSolver

!syntax inputs /Solver/MFEMGMRESSolver

!syntax children /Solver/MFEMGMRESSolver

!if-end!

!else
!include mfem/mfem_warning.md
