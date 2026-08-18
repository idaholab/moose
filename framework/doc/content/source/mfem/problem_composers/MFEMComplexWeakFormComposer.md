# MFEMComplexWeakFormComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMComplexWeakFormComposer` is the builder class for `ComplexEquationSystemProblemOperator`.
when called by an [MFEMSteady.md] executioner object.

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposer

!syntax parameters /ProblemComposer/MFEMComplexWeakFormComposer

!syntax inputs /ProblemComposer/MFEMComplexWeakFormComposer

!syntax children /ProblemComposer/MFEMComplexWeakFormComposer

!if-end!

!else
!include mfem/mfem_warning.md
