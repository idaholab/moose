# MFEMLineVariableValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs interpolation of real scalar and vector variables
along a specified line using MFEM's
[`FindPointsGSLIB`](https://mfem.org/howto/findpts/) `gslib` interpolation.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/line_value_sampler/line_value_sampler_diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMLineVariableValueSampler

!syntax inputs /VectorPostprocessors/MFEMLineVariableValueSampler

!syntax children /VectorPostprocessors/MFEMLineVariableValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
