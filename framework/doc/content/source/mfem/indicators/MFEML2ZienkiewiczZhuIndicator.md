# MFEML2ZienkiewiczZhuIndicator

!if! function=hasCapability('mfem')

## Summary

Child class of [MFEMIndicator.md] that supplies
an `mfem::L2ZienkiewiczZhuEstimator` to the [MFEMRefinementMarker.md].

## Overview

This class implements `createEstimator()`. If the user doesn't supply an FESpace for the
discontinuous flux with [!param](/Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator/flux_fespace),
or for the smoothed flux with
[!param](/Adaptivity/Indicators/MFEML2ZienkiewiczZhuIndicator/smooth_flux_fespace), then the method
creates and stores a default space compatible with the flux of the chosen kernel. For a variable of
order $p$, the defaults are:

- for the gradient of [MFEMDiffusionKernel.md], L2 and H1 spaces of order $p$ for the discontinuous
  and smoothed fluxes respectively, with one component per space dimension;
- for the stress of [MFEMLinearElasticityKernel.md], L2 and H1 spaces of order $p$ with
  `dim(dim+1)/2` components, holding the symmetric stress tensor in Voigt notation;
- for the curl of [MFEMCurlCurlKernel.md] in 2D, which is a scalar, scalar L2 and H1 spaces of
  order $p$;
- for the curl of [MFEMCurlCurlKernel.md] in 3D, a Raviart-Thomas space of order $p-1$, which holds
  the curl of a Nedelec field of order $p$, for the discontinuous flux, and a Nedelec space of order
  $p$ for the smoothed flux.

FESpaces supplied by the user must be compatible with the flux of the chosen kernel in the same way,
and must be defined on the same mesh used by the variable associated with the chosen kernel.

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
