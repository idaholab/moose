# MFEMPointValueSampler

!if! function=hasCapability('mfem')

## Summary

Class which can interpolate scalar and vector quantities at a set of specified
points.

!syntax description /VectorPostprocessors/MFEMPointValueSampler

## Overview

This class performs accurate interpolation using MFEM's `FindPointsGSLIB`
`gslib` integration.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/point_value_sampler/point_value_sampler_diffusion.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMPointValueSampler

!syntax inputs /VectorPostprocessors/MFEMPointValueSampler

!syntax children /VectorPostprocessors/MFEMPointValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
