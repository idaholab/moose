# MFEMTimeDependentWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMTimeDependentWeakFormProblemComposer` is the builder class for `TimeDependentEquationSystemProblemOperator`.

!listing test/tests/mfem/problemcomposers/prob_op_block_heattransfer.i block=Problem ProblemComposers

!listing test/tests/mfem/problemcomposers/prob_op_mfem_multiple_timesequences.i block=Problem ProblemComposers

!syntax parameters /ProblemComposers/MFEMTimeDependentWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMTimeDependentWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMTimeDependentWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
