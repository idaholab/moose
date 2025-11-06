# MFEMDotProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMDotProductAux

## Overview

AuxKernel for calculating the inner product of two vector fields and projecting onto an L2 finite element space mfem auxvariable.

!equation
s u\cdot v, \,\,\,

where $u$ and $v$ are the two fields and $s(x)$ is an optional scaling.

## Example Input File Syntax

!listing mfem/kernels/curlcurl.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMDotProductAux

!syntax inputs /AuxKernels/MFEMDotProductAux

!syntax children /AuxKernels/MFEMDotProductAux

!if-end!

!else
!include mfem/mfem_warning.md
