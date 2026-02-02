# MFEMPointValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs accurate interpolation of scalar and vector quantities at a
set of specified points using MFEM's `FindPointsGSLIB` `gslib` interpolation.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/point_value_sampler/point_value_sampler_diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMPointValueSampler

!syntax inputs /VectorPostprocessors/MFEMPointValueSampler

!syntax children /VectorPostprocessors/MFEMPointValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
