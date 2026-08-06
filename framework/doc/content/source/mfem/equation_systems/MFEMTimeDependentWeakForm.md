# MFEMTimeDependentWeakForm

!if! function=hasCapability('mfem')

The `MFEMTimeDependentWeakForm` is a MOOSE object responsible for the construction of an initialised
MFEM [TimeDependentEquationSystem](source/mfem/equation_systems/TimeDependentEquationSystem.md),
based on the set of kernels and boundary conditions provided by the user. If no `MFEMWeakFormBase`
object is specified by the user in a real problem using an `MFEMTransient` executioner, a default
`MFEMTimeDependentWeakForm` object will be created to set up a `TimeDependentEquationSystem` using
all kernels and boundary conditions added in the input file.

This class is intended to help separate out MOOSE-specific setup from the MFEM assembly of the
linear or nonlinear system used downstream in MFEM solvers.

!if-end!

!else
!include mfem/mfem_warning.md
