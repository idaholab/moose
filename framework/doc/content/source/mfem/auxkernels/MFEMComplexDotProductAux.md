# MFEMComplexDotProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMComplexDotProductAux

## Overview

AuxKernel for calculating the inner product of two complex vector fields and projecting onto an L2 finite element space mfem complex auxvariable.

!equation
s u\cdot v, \,\,\,

where $u$ and $v$ are the two fields and $s(x)$ is an optional complex scaling.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexDotProductAux

!syntax inputs /AuxKernels/MFEMComplexDotProductAux

!syntax children /AuxKernels/MFEMComplexDotProductAux

!if-end!

!else
!include mfem/mfem_warning.md
