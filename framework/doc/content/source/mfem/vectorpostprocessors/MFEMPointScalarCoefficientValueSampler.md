# MFEMPointScalarCoefficientValueSampler

!if! function=hasCapability('mfem')

## Overview

This vector postprocessor evaluates a real scalar MFEM coefficient at specified
points. Unlike an MFEM variable, a coefficient is a function used by finite
element operators and need not belong to a finite element space. The sampler
therefore evaluates the coefficient directly in the owning element selected by
MFEM's [`FindPointsGSLIB`](https://mfem.org/howto/findpts/) point search.

An arbitrary coefficient may be discontinuous across an element boundary, and
MFEM coefficients do not provide general continuity metadata. When a sample is
classified as lying on an element boundary, the sampler issues a warning and
returns the value from the element selected by GSLIB. The variable sampler's
`side_interpolation_type` arithmetic and harmonic averages do not apply to
coefficients.

Quadrature-function-backed coefficients are defined only at the quadrature
points configured by their quadrature rule. They cannot be evaluated at
arbitrary sample points and are rejected by this sampler.

## Example Input File Syntax

!listing mfem/vectorpostprocessors/coefficient_value_sampler/coefficient_value_sampler.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/MFEMPointScalarCoefficientValueSampler

!syntax inputs /VectorPostprocessors/MFEMPointScalarCoefficientValueSampler

!syntax children /VectorPostprocessors/MFEMPointScalarCoefficientValueSampler

!if-end!

!else
!include mfem/mfem_warning.md
