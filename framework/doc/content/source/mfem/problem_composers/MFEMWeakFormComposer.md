#  MFEMWeakFormComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMWeakFormComposer` is the builder class for `EquationSystemProblemOperator`.
when called by an [MFEMSteady.md] executioner object.

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposer

!syntax parameters /ProblemComposer/MFEMWeakFormComposer

!syntax inputs /ProblemComposer/MFEMWeakFormComposer

!syntax children /ProblemComposer/MFEMWeakFormComposer

!if-end!

!else
!include mfem/mfem_warning.md
