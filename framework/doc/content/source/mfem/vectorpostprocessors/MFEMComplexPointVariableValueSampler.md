# MFEMComplexPointVariableValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs interpolation of complex scalar and vector quantities at a
set of specified points using MFEM's
[`FindPointsGSLIB`](https://mfem.org/howto/findpts/) `gslib` interpolation.
The real and imaginary parts of each sampled component are output as separate columns.

## Example Input File Syntax

!listing mfem/variables/complex_aux_recovery.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMComplexPointVariableValueSampler

!syntax inputs /VectorPostprocessors/MFEMComplexPointVariableValueSampler

!syntax children /VectorPostprocessors/MFEMComplexPointVariableValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
