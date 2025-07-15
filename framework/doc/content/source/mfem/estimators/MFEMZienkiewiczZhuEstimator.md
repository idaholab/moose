# MFEMZienkiewiczZhuEstimator

!if! function=hasCapability('mfem')

## Summary

Child class of [MFEMEstimator](source/mfem/estimators/MFEMEstimator.md) that supplies
the `mfem::L2ZienkiewiczZhuEstimator` to the [Refiner](source/mfem/refiners/MFEMThresholdRefiner.md)

## Overview

This class implements the `createEstimator()`. This creates and stores an L2 finite element collection
(and an associated finite element space) for the discontinuous flux, as well as an H1 finite element
collection for the smoothed flux.

We can then construct the `L2ZienkiewiczZhuEstimator` object using the bilinear form integrator
associated with the kernel, the grid function associated with the variable, and the two finite
element spaces we create for the fluxes.

We can access this object using the `getEstimator()` method.

!if-end!

!else
!include mfem/mfem_warning.md
