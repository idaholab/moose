# MFEMComplexWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexWeakFormProblemComposer` is the builder class for `ComplexEquationSystemProblemOperator`.

## Input File Syntax

!syntax parameters /ProblemComposer/MFEMComplexWeakFormProblemComposer

!syntax inputs /ProblemComposer/MFEMComplexWeakFormProblemComposer

!syntax children /ProblemComposer/MFEMComplexWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
