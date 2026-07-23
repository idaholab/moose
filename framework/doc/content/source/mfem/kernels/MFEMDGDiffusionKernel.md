# MFEMDGDiffusionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k\vec\nabla u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $u, v \in H^1$ and $k$ is a scalar diffusivity coefficient.

This term arises from the weak form of the Laplacian operator

!equation
- \vec\nabla \cdot \left( k \vec\nabla u \right)

This kernel complements the [regular diffusion kernel](source/mfem/kernels/MFEMDiffusionKernel.md), except
this is applied to the bilinear form via the method `AddInteriorFaceIntegrator`.
`createBFIntegrator()` returns an `mfem::DGDiffusionIntegrator` ([see mfem docs for more info](https://docs.mfem.org/html/classmfem_1_1DGDiffusionIntegrator.html)).

## Example Input File Syntax

!listing mfem/kernels/dg_diffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMDGDiffusionKernel

!syntax inputs /Kernels/MFEMDGDiffusionKernel

!syntax children /Kernels/MFEMDGDiffusionKernel

!if-end!

!else
!include mfem/mfem_warning.md