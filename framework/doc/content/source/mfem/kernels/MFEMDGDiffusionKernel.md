# MFEMDGDiffusionKernel

!if! function=hasCapability('mfem')

## Overview

This kernel complements the [regular diffusion kernel](source/mfem/kernels/MFEMDiffusionKernel.md), except
this is applied to the bilinear form via the method `AddInteriorFaceIntegrator()`.
`createBFIntegrator()` returns an [`mfem::DGDiffusionIntegrator`](https://docs.mfem.org/html/classmfem_1_1DGDiffusionIntegrator.html).

## Example Input File Syntax

!listing mfem/kernels/dg_diffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMDGDiffusionKernel

!syntax inputs /Kernels/MFEMDGDiffusionKernel

!syntax children /Kernels/MFEMDGDiffusionKernel

!if-end!

!else
!include mfem/mfem_warning.md