# MFEMTimeDependentWeakFormComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMTimeDependentWeakFormComposer` is the builder class for `TimeDependentEquationSystemProblemOperator` when called by an [MFEMTransient.md] executioner object.

!listing test/tests/mfem/problemcomposers/prob_op_block_heattransfer.i block=Problem ProblemComposer

!listing test/tests/mfem/problemcomposers/prob_op_mfem_multiple_timesequences.i block=Problem ProblemComposer

!syntax parameters /ProblemComposer/MFEMTimeDependentWeakFormComposer

!syntax inputs /ProblemComposer/MFEMTimeDependentWeakFormComposer

!syntax children /ProblemComposer/MFEMTimeDependentWeakFormComposer

!if-end!

!else
!include mfem/mfem_warning.md
