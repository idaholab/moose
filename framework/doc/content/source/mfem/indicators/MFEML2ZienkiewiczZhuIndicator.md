# MFEML2ZienkiewiczZhuIndicator

!if! function=hasCapability('mfem')

## Summary

Child class of [MFEMIndicator.md] that supplies
an `mfem::L2ZienkiewiczZhuEstimator` to the [MFEMRefinementMarker.md].

## Overview

This class implements `createEstimator()`. If the user doesn't supply an FESpace for the smoothed flux
with [!param](/Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator/smooth_flux_fespace), then the
method creates and stores an H1 finite element collection. Similarly, if the user doesn't supply an
FESpace for the discontinuous flux with
[!param](/Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator/flux_fespace), then the method creates
and stores an L2 finite element collection. Both default spaces have the order of the variable and one
component per component of the flux of the chosen kernel: the space dimension for the gradient of
[MFEMDiffusionKernel.md], `dim(dim+1)/2` for the stress of [MFEMLinearElasticityKernel.md] (a
symmetric tensor in Voigt notation), and one for the curl of [MFEMCurlCurlKernel.md] in 2D. In 3D, an L2
space cannot hold the curl of a Nedelec field, so a curl-curl kernel requires
[!param](/Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator/flux_fespace). FESpaces supplied by the
user must have the same number of components as the flux, and must be defined on the same mesh used by
the variable associated with the chosen kernel.

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
