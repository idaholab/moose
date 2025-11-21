# MFEMComplexCrossProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMComplexCrossProductAux

## Overview

AuxKernel for calculating the cross product of two complex vector fields and projecting onto an L2 vector finite element space complex mfem auxvariable.

!equation
s(x)(u\times v), \,\,\,

where $u$ and $v$ are the two complex vector fields and $s(x)$ is an optional complex scaling.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexCrossProductAux

!syntax inputs /AuxKernels/MFEMComplexCrossProductAux

!syntax children /AuxKernels/MFEMComplexCrossProductAux

!if-end!

!else
!include mfem/mfem_warning.md
