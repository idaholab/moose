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

This one is much the same as the [regular diffusion kernel](source/mfem/kernels/MFEMDiffusionKernel.md), except
this is applied to the bilinear form via the method `AddInteriorFaceIntegrator`.

## Example Input File Syntax

!listing mfem/kernels/dg_diffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMDGDiffusionKernel

!syntax inputs /Kernels/MFEMDGDiffusionKernel

!syntax children /Kernels/MFEMDGDiffusionKernel

!if-end!

!else
!include mfem/mfem_warning.md