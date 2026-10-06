# MFEMComplexWeakForm

!if! function=hasCapability('mfem')

## Overview

`MFEMWeakFormBase` is the base class of classes that build initialiseed [EquationSystem.md]
objects, through the method `createEquationSystem()`. The weak form object itself is created by
[MFEMProblem.md].

If no `MFEMWeakFormBase` object is specified by the user in an `MFEMProblem`, a single default
`MFEMWeakFormBase` derived object will be created using all kernels and boundary conditions added
in the input file. The type of the default `MFEMWeakFormBase` derived object will be either an
`MFEMWeakForm`, `MFEMComplexWeakForm`, `MFEMTimeDependentWeakForm`, or `MFEMEigenproblemWeakForm`
depending on whether the problem is real or complex, whether a steady or transient executioner is
applied, and whether the problem is solving an eigenproblem or not.

This class is intended to help separate out MOOSE-specific setup from the MFEM assembly of the
linear or nonlinear system used downstream in MFEM solvers.

!if-end!

!else
!include mfem/mfem_warning.md
