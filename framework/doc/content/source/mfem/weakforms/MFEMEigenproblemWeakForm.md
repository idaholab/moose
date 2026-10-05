# MFEMEigenproblemWeakForm

!if! function=hasCapability('mfem')

## Overview
The `MFEMEigenproblemWeakForm` is a MOOSE object responsible for the construction of an initialised
MFEM [EigenproblemEquationSystem.md], based
on the set of kernels and boundary conditions provided by the user. If no `MFEMWeakFormBase` object
is specified by the user in a real `MFEMEigenproblem` using an `MFEMSteady` executioner, a default
`MFEMEigenproblemWeakForm` object will be created to set up an `EigenproblemEquationSystem` using
all kernels and boundary conditions added in the input file.

!syntax parameters /WeakForms/MFEMEigenproblemWeakForm

!syntax inputs /WeakForms/MFEMEigenproblemWeakForm

!syntax children /WeakForms/MFEMEigenproblemWeakForm

!if-end!

!else
!include mfem/mfem_warning.md
