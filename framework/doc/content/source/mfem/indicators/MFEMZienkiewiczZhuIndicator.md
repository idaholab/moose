# MFEMZienkiewiczZhuIndicator

!if! function=hasCapability('mfem')

## Summary

Child class of [MFEMIndicator.md] that supplies
an `mfem::L2ZienkiewiczZhuEstimator` to the [MFEMRefinementMarker.md].

## Overview

This class implements `createEstimator()`. This creates and stores an L2 finite element collection
(and an associated finite element space) for the discontinuous flux, as well as an H1 finite element
collection for the smoothed flux.

We can then construct the `L2ZienkiewiczZhuEstimator` object using the bilinear form integrator
associated with the chosen kernel, the grid function associated with the variable, and the two finite
element spaces we created for the fluxes. This object needs the underlying `mfem::BilinearFormIntegrator`
to implement the methods `ComputeElementFlux()` and `ComputeFluxEnergy()`. Practically speaking, this
means the only kernels we can attach this indicator to are [MFEMDiffusionKernel.md],
[MFEMCurlCurlKernel.md] and [MFEMLinearElasticityKernel.md]

This object is accessed once (by an `MFEMRefinementMarker`) using the `getEstimator()` method.

## Example Input File Syntax

!listing mfem/kernels/diffusion_amr.i block=Adaptivity

!syntax parameters /Adaptivity/Indicators/MFEMZienkiewiczZhuIndicator

!syntax inputs /Adaptivity/Indicators/MFEMZienkiewiczZhuIndicator

!syntax children /Adaptivity/Indicators/MFEMZienkiewiczZhuIndicator

!if-end!

!else
!include mfem/mfem_warning.md
