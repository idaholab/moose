# MFEMComplexEigenWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Overview

`MFEMComplexEigenWeakFormProblemComposer` is the builder class for `ComplexEigenproblemESProblemOperator`.

## Input File Syntax

!syntax parameters /ProblemComposers/MFEMComplexEigenWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMComplexEigenWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMComplexEigenWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
