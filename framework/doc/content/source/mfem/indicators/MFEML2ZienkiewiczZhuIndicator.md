# MFEML2ZienkiewiczZhuIndicator

!if! function=hasCapability('mfem')

## Summary

Child class of [MFEMIndicator.md] that supplies
an `mfem::L2ZienkiewiczZhuEstimator` to the [MFEMRefinementMarker.md].

## Overview

This class implements `createEstimator()`. If the user doesn't supply an FESpace for the smoothed flux,
then the method creates and stores an H1 finite element collection. Similarly, if the user doesn't
supply an FESpace for the discontinuous flux, then the method creates and stores an L2 finite element
collection. Be aware that the vector dimensions for each of these should match the space dimension and
that these spaces must be defined on the same mesh used by the variable associated with the chosen kernel.

We can then construct the `mfem::L2ZienkiewiczZhuEstimator` object using the bilinear form integrator
associated with the chosen kernel, the grid function associated with the variable, and the two finite
element spaces we created for the fluxes. This object needs the underlying `mfem::BilinearFormIntegrator`
to implement the method `ComputeElementFlux()`. Practically speaking, this
means the only kernels we can attach this indicator to are [MFEMDiffusionKernel.md],
[MFEMCurlCurlKernel.md] and [MFEMLinearElasticityKernel.md].

This object is accessed once (by an `MFEMRefinementMarker`) using the `getEstimator()` method.

## Example Input File Syntax

!listing mfem/kernels/diffusion_amr.i block=Adaptivity

!syntax parameters /Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator

!syntax inputs /Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator

!syntax children /Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator

!if-end!

!else
!include mfem/mfem_warning.md
