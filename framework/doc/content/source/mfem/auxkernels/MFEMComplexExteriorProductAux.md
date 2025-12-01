# MFEMComplexExteriorProductAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMComplexExteriorProductAux

## Overview

AuxKernel for calculating the exterior product of two complex vector fields and projecting onto an L2 vector finite element space complex mfem auxvariable.

!equation
s (u\wedge v*), \,\,\,

where $u$ and $v$ are the two complex vector fields and $s$ is an optional complex scaling.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexExteriorProductAux

!syntax inputs /AuxKernels/MFEMComplexExteriorProductAux

!syntax children /AuxKernels/MFEMComplexExteriorProductAux

!if-end!

!else
!include mfem/mfem_warning.md
