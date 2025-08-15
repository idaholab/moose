# MFEMValueSamplerBase

!if! function=hasCapability('mfem')

## Summary

Base class which can interpolate scalar and vector quantities at a set of
points. Derived classes control where those points come from, for example a
list of points or a line.

## Overview

This class performs accurate interpolation using MFEM's `FindPointsGSLIB`
`gslib` integration.

!if-end!

!else
!include mfem/mfem_warning.md
