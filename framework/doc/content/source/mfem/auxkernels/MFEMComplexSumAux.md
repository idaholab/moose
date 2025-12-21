# MFEMComplexSumAux

!if! function=hasCapability('mfem')

## Overview

AuxKernel for calculating the sum of two or more complex MFEM variables that are defined on the same finite
element space, and storing the result in another. All variables may be (optionally) scaled by a complex
scalar constant prior to addition.

!equation
u = \sum_i \lambda_i v_i.

where $u$ and $\{v_i\}$ are defined on the same FE space, and $\{\lambda_i\}$ are complex scalar
constants.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexSumAux

!syntax inputs /AuxKernels/MFEMComplexSumAux

!syntax children /AuxKernels/MFEMComplexSumAux

!if-end!

!else
!include mfem/mfem_warning.md
