# SteadyProblemComposer

!if! function=hasCapability('mfem')

## Summary

`SteadyProblemComposer` is the builder class for all steady equation system [ProblemOperator.md]s
when called by an [MFEMSteady.md] executioner object. It builds one of `EquationSystemProblemOperator`
,`EigenproblemESProblemOperator` or `ComplexEquationSystemProblemOperator` depending on the problem's type.

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposer

!syntax parameters /MFEMProblemComposer/AddMFEMProblemComposerAction

!if-end!

!else
!include mfem/mfem_warning.md
