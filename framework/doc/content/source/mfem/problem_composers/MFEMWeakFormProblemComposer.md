#  MFEMWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMWeakFormProblemComposer` is the builder class for `EquationSystemProblemOperator`.

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposers

!syntax parameters /ProblemComposers/MFEMWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
