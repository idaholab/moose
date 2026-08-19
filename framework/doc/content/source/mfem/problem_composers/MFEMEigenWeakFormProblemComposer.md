# MFEMEigenWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMEigenWeakFormProblemComposer` is the builder class for `EigenproblemESProblemOperator`.
when called by an [MFEMSteady.md] executioner object.

!syntax parameters /ProblemComposer/MFEMEigenWeakFormProblemComposer

!syntax inputs /ProblemComposer/MFEMEigenWeakFormProblemComposer

!syntax children /ProblemComposer/MFEMEigenWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
