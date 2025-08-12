# MFEMVectorProjectAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMVectorProjectAux

## Overview

AuxKernel for projecting a vector coefficient onto an MFEM variable.

## Example Input File Syntax

!listing 2DVectorCurrentProjection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMVectorProjectAux

!syntax inputs /AuxKernels/MFEMVectorProjectAux

!syntax children /AuxKernels/MFEMVectorProjectAux

!if-end!

!else
!include mfem/mfem_warning.md
