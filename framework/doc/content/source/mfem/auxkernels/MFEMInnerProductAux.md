# MFEMInnerProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMInnerProductAux

## Overview

AuxKernel for calculating the inner product of two vector fields and projecting
the result onto an MFEM auxiliary variable in an L2 finite element space.

!equation
s \vec u \cdot \vec v

where $\vec u$ and $\vec v$ are the two vector fields and $s$ is an optional
scaling coefficient.

## Example Input File Syntax

!listing mfem/kernels/curlcurl.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMInnerProductAux

!syntax inputs /AuxKernels/MFEMInnerProductAux

!syntax children /AuxKernels/MFEMInnerProductAux

!if-end!

!else
!include mfem/mfem_warning.md
