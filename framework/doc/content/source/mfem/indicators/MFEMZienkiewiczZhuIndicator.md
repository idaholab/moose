# MFEMZienkiewiczZhuIndicator

!if! function=hasCapability('mfem')

## Summary

Child class of [MFEMIndicator](source/mfem/indicators/MFEMIndicator.md) that supplies
the `mfem::L2ZienkiewiczZhuEstimator` to the [MFEMRefinementMarker](source/mfem/markers/MFEMRefinementMarker.md)

## Overview

This class implements the `createEstimator()`. This creates and stores an L2 finite element collection
(and an associated finite element space) for the discontinuous flux, as well as an H1 finite element
collection for the smoothed flux.

We can then construct the `L2ZienkiewiczZhuEstimator` object using the bilinear form integrator
associated with the kernel, the grid function associated with the variable, and the two finite
element spaces we create for the fluxes.

This object is accessed once (by an `MFEMRefinementMarker`) this object using the `getEstimator()` method.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion_amr.i block=Adaptivity

!if-end!

!else
!include mfem/mfem_warning.md
