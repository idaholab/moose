# MFEMWeakForm

!if! function=hasCapability('mfem')

The `MFEMWeakForm` is a MOOSE object responsible for the construction of an initialised MFEM
[EquationSystem](source/mfem/equation_systems/EquationSystem.md), based on the set of kernels and
boundary conditions provided by the user. If no `MFEMWeakFormBase` object is specified by the user
in a real problem using an `MFEMSteady` executioner, a default `MFEMWeakForm` object will
be created to set up an `EquationSystem` using all kernels and boundary conditions added in the
input file.

This class is intended to help seperate out MOOSE-specific setup from the MFEM assembly of the
linear or nonlinear system used downstream in MFEM solvers.

!if-end!

!else
!include mfem/mfem_warning.md
