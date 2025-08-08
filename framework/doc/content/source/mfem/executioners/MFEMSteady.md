# MFEMSteady

!if! function=hasCapability('mfem')

!syntax description /Executioner/MFEMSteady

## Overview

`MFEMSteady` is the `Executioner` class used to solve time independent MFEM finite element problems,
calling the [`MFEMProblemSolve`](MFEMProblemSolve.md) solve object to execute one or more MFEM
`ProblemOperators`.

As in all `Executioner` classes using the [MFEMProblemSolve.md] solve object, the desired device and assembly
level to use during problem set-up and solution can be selected.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Executioner

!syntax parameters /Executioner/MFEMSteady

!syntax inputs /Executioner/MFEMSteady

!syntax children /Executioner/MFEMSteady

!if-end!

!else
!include mfem/mfem_warning.md
