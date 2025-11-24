# MFEMCrossProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMCrossProductAux

## Overview

AuxKernel for calculating the cross product of two vector fields and projecting
the result onto an MFEM auxiliary variable in an L2 vector finite element space.

!equation
s \vec u \times \vec v

where $\vec u$ and $\vec v$ are the two vector fields and $s$ is an optional
scaling coefficient.

## Example Input File Syntax

!listing mfem/auxkernels/crossproduct.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMCrossProductAux

!syntax inputs /AuxKernels/MFEMCrossProductAux

!syntax children /AuxKernels/MFEMCrossProductAux

!if-end!

!else
!include mfem/mfem_warning.md
