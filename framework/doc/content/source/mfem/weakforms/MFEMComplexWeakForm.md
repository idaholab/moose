# MFEMComplexWeakForm

!if! function=hasCapability('mfem')

## Overview

The `MFEMComplexWeakForm` is a MOOSE object responsible for the construction of an initialised MFEM
`ComplexEquationSystem`, based on the set of kernels and boundary conditions provided by the user.
If no `MFEMWeakFormBase` object is specified by the user in a complex problem using an `MFEMSteady`
executioner, a default `MFEMComplexWeakForm` object will be created to set up a
`ComplexEquationSystem` using all kernels and boundary conditions added in the input file.

## Example Input File Syntax

!listing test/tests/mfem/weakforms/complex_weakform.i block=WeakForms

!syntax parameters /WeakForms/MFEMComplexWeakForm

!syntax inputs /WeakForms/MFEMComplexWeakForm

!syntax children /WeakForms/MFEMComplexWeakForm

!if-end!

!else
!include mfem/mfem_warning.md
