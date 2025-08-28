# MFEMSumAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMSumAux

## Overview

AuxKernel for calculating the sum of two MFEM variables that are defined on the same finite element
space, and storing the result in a third. Both variables may be (optionally) scaled by a real scalar
constant prior to addition.

!equation
u = \lambda_1 v_1 + \lambda_2 v_2.

where $u, v_1, v_2$ are defined on the same FE space, and $\lambda_1, \lambda_2$ are real scalar
constants.

## Example Input File Syntax

!listing mfem/submeshes/cut_closed_coil.i block=AuxKernels

!syntax parameters /AuxKernels/MFEMSumAux

!syntax inputs /AuxKernels/MFEMSumAux

!syntax children /AuxKernels/MFEMSumAux

!if-end!

!else
!include mfem/mfem_warning.md
