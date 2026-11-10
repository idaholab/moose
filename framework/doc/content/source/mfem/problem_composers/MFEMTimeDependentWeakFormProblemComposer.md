# MFEMTimeDependentWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Overview

`MFEMTimeDependentWeakFormProblemComposer` is the builder class for `TimeDependentEquationSystemProblemOperator`.

## Example Input File Syntax

!listing test/tests/mfem/problemcomposers/explicit_composer_heat_transfer.i block=Problem ProblemComposers

!syntax parameters /ProblemComposers/MFEMTimeDependentWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMTimeDependentWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMTimeDependentWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
