# MFEMPointVariableValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs interpolation of real scalar and vector variables at a
set of specified points using MFEM's
[`FindPointsGSLIB`](https://mfem.org/howto/findpts/) `gslib` interpolation.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/point_value_sampler/point_value_sampler_diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMPointVariableValueSampler

!syntax inputs /VectorPostprocessors/MFEMPointVariableValueSampler

!syntax children /VectorPostprocessors/MFEMPointVariableValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
