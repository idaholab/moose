# MFEMComplexVariableLineValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs interpolation of complex scalar and vector variables
along a specified line using MFEM's
[`FindPointsGSLIB`](https://mfem.org/howto/findpts/) `gslib` interpolation.
The real and imaginary parts of each sampled component are output as separate columns.

## Example Input File Syntax

!listing mfem/complex/complex.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMComplexVariableLineValueSampler

!syntax inputs /VectorPostprocessors/MFEMComplexVariableLineValueSampler

!syntax children /VectorPostprocessors/MFEMComplexVariableLineValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
