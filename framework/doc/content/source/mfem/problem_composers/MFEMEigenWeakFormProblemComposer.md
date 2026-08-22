# MFEMEigenWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMEigenWeakFormProblemComposer` is the builder class for `EigenproblemESProblemOperator`.

!syntax parameters /ProblemComposers/MFEMEigenWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMEigenWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMEigenWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
