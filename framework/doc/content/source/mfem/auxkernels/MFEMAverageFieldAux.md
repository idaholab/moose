# MFEMAverageFieldAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMAverageFieldAux

## Overview

AuxKernel for calculating the running time average of an MFEM variable during a transient simulation projected onto an AuxVariable.

## Example Input File Syntax

!listing mfem/kernels/heattransfer.i block=/AuxKernels

!syntax parameters /AuxKernels/MFEMAverageFieldAux

!syntax inputs /AuxKernels/MFEMAverageFieldAux

!syntax children /AuxKernels/MFEMAverageFieldAux

!if-end!

!else
!include mfem/mfem_warning.md
