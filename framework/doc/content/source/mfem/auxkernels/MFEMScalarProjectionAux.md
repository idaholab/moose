# MFEMScalarProjectionAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMScalarProjectionAux

## Overview

AuxKernel for projecting a scalar coefficient onto an MFEM variable.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMScalarProjectionAux

!syntax inputs /AuxKernels/MFEMScalarProjectionAux

!syntax children /AuxKernels/MFEMScalarProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
