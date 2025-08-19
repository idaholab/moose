# MFEMLineValueSampler

!if! function=hasCapability('mfem')

## Summary

Class which can interpolate scalar and vector quantities along a specified
line.

!syntax description /VectorPostprocessors/MFEMLineValueSampler

## Overview

This class performs accurate interpolation using MFEM's `FindPointsGSLIB`
`gslib` integration.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/point_value_sampler/point_value_sampler_diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMLineValueSampler

!syntax inputs /VectorPostprocessors/MFEMLineValueSampler

!syntax children /VectorPostprocessors/MFEMLineValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
