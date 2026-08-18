# MFEMEigenWeakFormComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMEigenWeakFormComposer` is the builder class for `EigenproblemESProblemOperator`.
when called by an [MFEMSteady.md] executioner object.

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposer

!syntax parameters /ProblemComposer/MFEMEigenWeakFormComposer

!syntax inputs /ProblemComposer/MFEMEigenWeakFormComposer

!syntax children /ProblemComposer/MFEMEigenWeakFormComposer

!if-end!

!else
!include mfem/mfem_warning.md
