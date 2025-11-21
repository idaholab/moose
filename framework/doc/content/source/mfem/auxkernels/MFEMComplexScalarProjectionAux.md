# MFEMComplexScalarProjectionAux

!if! function=hasCapability('mfem')

## Summary

!syntax description /AuxKernels/MFEMComplexScalarProjectionAux

## Overview

AuxKernel for projecting a pair of scalar coefficients onto a complex scalar auxiliary variable
in, e.g., $H^1$ or $L^2$.

## Input File Syntax

!syntax parameters /AuxKernels/MFEMComplexScalarProjectionAux

!syntax inputs /AuxKernels/MFEMComplexScalarProjectionAux

!syntax children /AuxKernels/MFEMComplexScalarProjectionAux

!if-end!

!else
!include mfem/mfem_warning.md
