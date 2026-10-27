# MFEMEigenWeakFormProblemComposer

!if! function=hasCapability('mfem')

## Overview

`MFEMEigenWeakFormProblemComposer` is the builder class for `EigenproblemESProblemOperator`.

## Input File Syntax

!syntax parameters /ProblemComposers/MFEMEigenWeakFormProblemComposer

!syntax inputs /ProblemComposers/MFEMEigenWeakFormProblemComposer

!syntax children /ProblemComposers/MFEMEigenWeakFormProblemComposer

!if-end!

!else
!include mfem/mfem_warning.md
