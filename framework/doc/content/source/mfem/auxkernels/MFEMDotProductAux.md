# MFEMDotProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMDotProductAux

## Overview

AuxKernel for calculating the inner product of two fields and projecting onto an L2 finite element space mfem auxvariable.

!equation
s(x)(u\cdot v), \,\,\,

where $u$ and $u$ are the two fields and $s(x)$ is an optional scaling.

## Example Input File Syntax

!listing mfem/kernels/curlcurl.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMDotProductAux

!syntax inputs /AuxKernels/MFEMDotProductAux

!syntax children /AuxKernels/MFEMDotProductAux

!if-end!

!else
!include mfem/mfem_warning.md
