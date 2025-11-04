# MFEMTransient

!if! function=hasCapability('mfem')

## Summary

!syntax description /Executioner/MFEMTransient

## Overview

`MFEMTransient` is the `Executioner` class used to solve time dependent MFEM finite element
problems, calling the [`MFEMProblemSolve`](MFEMProblemSolve.md) solve object to execute one or more
MFEM `TimeDomainProblemOperators`.

As in all `Executioner` classes using the [MFEMProblemSolve.md] solve object,
the desired device and assembly level to use during problem set-up and solution can be selected.

Currently, only simulations with an implicit backwards Euler time integrator are supported.
However, the [TimeStepper](/TimeSteppers/index.md) system is fully supported providing wide choice for the timestep(s) `dt`.

## Example Input File Syntax

!listing test/tests/mfem/kernels/heattransfer.i block=Executioner

!syntax parameters /Executioner/MFEMTransient

!syntax inputs /Executioner/MFEMTransient

!syntax children /Executioner/MFEMTransient

!if-end!

!else
!include mfem/mfem_warning.md
