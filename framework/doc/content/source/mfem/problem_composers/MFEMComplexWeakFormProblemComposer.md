# MFEMComplexWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Summary

`MFEMComplexWeakFormProblemComposer` is the builder class for `ComplexEquationSystemProblemOperator`.

## Input File Syntax

!syntax parameters /ProblemComposers/MFEMComplexWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMComplexWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMComplexWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
