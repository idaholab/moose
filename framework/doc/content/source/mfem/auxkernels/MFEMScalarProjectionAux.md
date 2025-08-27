# MFEMScalarProjectionAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMScalarProjectionAux

## Overview

AuxKernel for projecting a scalar coefficient onto a scalar auxiliary variable
in, e.g., $H^1$ or $L^2$.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMScalarProjectionAux

!syntax inputs /AuxKernels/MFEMScalarProjectionAux

!syntax children /AuxKernels/MFEMScalarProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
