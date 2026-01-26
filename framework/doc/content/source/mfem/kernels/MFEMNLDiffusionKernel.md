# MFEMNLDiffusionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(u\vec\nabla u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $u, v \in H^1$ 

This term arises from the weak form of the Laplacian operator

!equation
- \vec\nabla \cdot \left( u \vec\nabla u \right)

## Example Input File Syntax

!listing mfem/kernels/nldiffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMDiffusionKernel

!syntax inputs /Kernels/MFEMDiffusionKernel

!syntax children /Kernels/MFEMDiffusionKernel

!if-end!

!else
!include mfem/mfem_warning.md
