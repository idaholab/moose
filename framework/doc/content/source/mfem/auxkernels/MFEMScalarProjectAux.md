# MFEMScalarProjectAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMScalarProjectAux

## Overview

AuxKernel for projecting a scalar coefficient onto an MFEM variable.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMScalarProjectAux

!syntax inputs /AuxKernels/MFEMScalarProjectAux

!syntax children /AuxKernels/MFEMScalarProjectAux

!if-end!

!else
!include mfem/mfem_warning.md
