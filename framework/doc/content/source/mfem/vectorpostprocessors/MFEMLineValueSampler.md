# MFEMLineValueSampler

!if! function=hasCapability('mfem')

## Overview

This class performs accurate interpolation of scalar and vector quantities
along a specified line using MFEM's `FindPointsGSLIB` `gslib` integration.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/point_value_sampler/point_value_sampler_diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMLineValueSampler

!syntax inputs /VectorPostprocessors/MFEMLineValueSampler

!syntax children /VectorPostprocessors/MFEMLineValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
