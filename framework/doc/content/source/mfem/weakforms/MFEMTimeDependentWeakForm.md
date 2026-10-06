# MFEMTimeDependentWeakForm

!if! function=hasCapability('mfem')

## Overview

The `MFEMTimeDependentWeakForm` is a MOOSE object responsible for the construction of an initialised
MFEM [TimeDependentEquationSystem.md],
based on the set of kernels and boundary conditions provided by the user. If no `MFEMWeakFormBase`
object is specified by the user in a real problem using an `MFEMTransient` executioner, a default
`MFEMTimeDependentWeakForm` object will be created to set up a `TimeDependentEquationSystem` using
all kernels and boundary conditions added in the input file.

## Example Input File Syntax

!listing test/tests/mfem/weakforms/transient_weakform.i block=WeakForms

!syntax parameters /WeakForms/MFEMTimeDependentWeakForm

!syntax inputs /WeakForms/MFEMTimeDependentWeakForm

!syntax children /WeakForms/MFEMTimeDependentWeakForm

!if-end!

!else
!include mfem/mfem_warning.md
