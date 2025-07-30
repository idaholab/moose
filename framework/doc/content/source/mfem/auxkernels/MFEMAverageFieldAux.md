# MFEMAverageFieldAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMAverageFieldAux

## Overview

AuxKernel for calculating the time weighted average value of a running MFEM variable during a transient simulation projected into an AuxVariable

## Example Input File Syntax

!listing HeatTransferAverageField.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMAverageFieldAux

!syntax inputs /AuxKernels/MFEMAverageFieldAux

!syntax children /AuxKernels/MFEMAverageFieldAux

!if-end!

!else
!include mfem/mfem_warning.md
