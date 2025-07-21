# MFEMProblemSolve

!if! function=hasCapability('mfem')

## Summary

Virtual base class for executioners used to solve MFEM FE problems.

## Overview

`MFEMProblemSolve` is a purely virtual base class for solve objects used to control the execution of
MFEM FE problems. Importantly, `MFEMProblemSolve` is used in `Executioner` classes intended for the
solution of problems using the MFEM FE backend, such as `MFEMTransient` and `MFEMSteady`, by
executing their corresponding `ProblemOperators`. `MFEMProblemSolve` also allows for selection of
the desired device and assembly strategy to use for the FE problem.

Provided MFEM and its dependencies have been built with support for GPU backends (e.g. `cuda` or
`hip`), they may be selected here to enable use of GPUs for solvers and during assembly.

Selection of partial assembly levels is also supported to improve performance on GPUs; full
description of the various assembly levels supported is available
 [here](https://mfem.org/performance/).

By default, legacy assembly of the FE problem on CPUs will be selected for robustness.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=Executioner

!listing test/tests/mfem/kernels/heattransfer.i block=Executioner

!if-end!

!else
!include mfem/mfem_warning.md
