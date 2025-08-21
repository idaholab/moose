# MFEMScalarTimeAverageAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMScalarTimeAverageAux

## Overview

AuxKernel for calculating the running time average of an MFEM variable during a transient simulation projected onto an AuxVariable.

## Example Input File Syntax

!listing mfem/kernels/heattransfer.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMScalarTimeAverageAux

!syntax inputs /AuxKernels/MFEMScalarTimeAverageAux

!syntax children /AuxKernels/MFEMScalarTimeAverageAux

!if-end!

!else
!include mfem/mfem_warning.md
