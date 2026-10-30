# MFEMTransient

!if! function=hasCapability('mfem')

## Overview

`MFEMTransient` is the `Executioner` class used to solve time dependent MFEM finite element
problems, calling the [MFEMProblemSolve.md] solve object to execute one or more
MFEM `TimeDependentProblemOperators`.

As in all `Executioner` classes using the [MFEMProblemSolve.md] solve object,
the desired device and assembly level to use during problem set-up and solution can be selected.

Currently, only simulations with an implicit backwards Euler time integrator are supported.
However, the [TimeStepper](/TimeSteppers/index.md) system is fully supported providing wide choice for the timestep(s) `dt`.

Throughout each time step, the problem time and the time of the MFEM coefficients are the time at
the end of the step, so that objects executed at `TIMESTEP_BEGIN`, MultiApps, transfers and the
solve all see the same time. A steady equation system, such as one executed by an
[MFEMWeakFormProblemComposer.md], is solved again at every step at that time:

!listing test/tests/mfem/problemcomposers/steady_composer_transient.i block=ProblemComposers Executioner

## Example Input File Syntax

!listing test/tests/mfem/kernels/heattransfer.i block=Executioner

!syntax parameters /Executioner/MFEMTransient

!syntax inputs /Executioner/MFEMTransient

!syntax children /Executioner/MFEMTransient

!if-end!

!else
!include mfem/mfem_warning.md
