# MFEMVariableLineValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs interpolation of real scalar and vector variables
along a specified line using MFEM's
[`FindPointsGSLIB`](https://mfem.org/howto/findpts/) `gslib` interpolation.

## Example Input File Syntax

!listing mfem/kernels/diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMVariableLineValueSampler

!syntax inputs /VectorPostprocessors/MFEMVariableLineValueSampler

!syntax children /VectorPostprocessors/MFEMVariableLineValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
