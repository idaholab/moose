# MFEMComplexWeakForm

!if! function=hasCapability('mfem')

The `MFEMComplexWeakForm` is a MOOSE object responsible for the construction of an initialised MFEM
`ComplexEquationSystem`, based on the set of kernels and boundary conditions provided by the user.
If no `MFEMWeakFormBase` object is specified by the user in a complex problem using an `MFEMSteady`
executioner, a default `MFEMComplexWeakForm` object will be created to set up a
`ComplexEquationSystem` using all kernels and boundary conditions added in the input file.

This class is intended to help separate out MOOSE-specific setup from the MFEM assembly of the
linear or nonlinear system used downstream in MFEM solvers.

!if-end!

!else
!include mfem/mfem_warning.md
