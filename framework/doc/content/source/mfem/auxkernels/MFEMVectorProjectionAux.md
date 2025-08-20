# MFEMVectorProjectionAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMVectorProjectionAux

## Overview

AuxKernel for projecting a vector coefficient onto an MFEM variable.

## Example Input File Syntax

!listing mfem/auxkernels/projection.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMVectorProjectionAux

!syntax inputs /AuxKernels/MFEMVectorProjectionAux

!syntax children /AuxKernels/MFEMVectorProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
