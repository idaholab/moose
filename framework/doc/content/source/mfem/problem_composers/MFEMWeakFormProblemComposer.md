#  MFEMWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMWeakFormProblemComposer` is the builder class for `EquationSystemProblemOperator`.
when called by an [MFEMSteady.md] executioner object.

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposer

!syntax parameters /ProblemComposer/MFEMWeakFormProblemComposer

!syntax inputs /ProblemComposer/MFEMWeakFormProblemComposer

!syntax children /ProblemComposer/MFEMWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
