# MFEMCrossProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMCrossProductAux

## Overview

AuxKernel for calculating the cross product of two vector fields and projecting onto an L2 vector finite element space mfem auxvariable.

!equation
s(x)(u\times v), \,\,\,

where $u$ and $u$ are the two vector fields and $s(x)$ is an optional scaling.

## Example Input File Syntax

!listing mfem/submeshes/av_magnetostatic.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMCrossProductAux

!syntax inputs /AuxKernels/MFEMCrossProductAux

!syntax children /AuxKernels/MFEMCrossProductAux

!if-end!

!else
!include mfem/mfem_warning.md
