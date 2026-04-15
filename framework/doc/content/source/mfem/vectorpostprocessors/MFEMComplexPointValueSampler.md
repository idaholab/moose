# MFEMComplexPointValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs accurate interpolation of complex scalar and vector quantities at a
set of specified points using MFEM's
[`FindPointsGSLIB`](https://mfem.org/howto/findpts/) `gslib` interpolation.
The real and imaginary parts of each sampled component are output as separate columns.

## Example Input File Syntax

!listing mfem/kernels/nl_heatconduction_complex_aux.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMComplexPointValueSampler

!syntax inputs /VectorPostprocessors/MFEMComplexPointValueSampler

!syntax children /VectorPostprocessors/MFEMComplexPointValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
