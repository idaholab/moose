# MFEMSumAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMSumAux

## Overview

AuxKernel for calculating the sum of two or more MFEM variables that are defined on the same finite
element space, and storing the result in another. All variables may be (optionally) scaled by a real
scalar constant prior to addition.

!equation
u = \sum_i \lambda_i v_i.

where $u$ and $\{v_i\}$ are defined on the same FE space, and $\{\lambda_i\}$ are real scalar
constants.

## Example Input File Syntax

!listing mfem/submeshes/cut_closed_coil.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMSumAux

!syntax inputs /AuxKernels/MFEMSumAux

!syntax children /AuxKernels/MFEMSumAux

!if-end!

!else
!include mfem/mfem_warning.md
