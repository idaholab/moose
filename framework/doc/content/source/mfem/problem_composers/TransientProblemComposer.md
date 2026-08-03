# TransientProblemComposer

!if! function=hasCapability('mfem')

## Summary

`TransientProblemComposer` is the builder class for `TimeDependentEquationSystemProblemOperator` when called by an [MFEMTransient.md] executioner object.

!listing test/tests/mfem/problemcomposers/prob_op_block_heattransfer.i block=Problem ProblemComposer

!listing test/tests/mfem/problemcomposers/prob_op_mfem_multiple_timesequences.i block=Problem ProblemComposer

!syntax parameters /MFEMProblemComposer/AddMFEMProblemComposerAction

!if-end!

!else
!include mfem/mfem_warning.md
