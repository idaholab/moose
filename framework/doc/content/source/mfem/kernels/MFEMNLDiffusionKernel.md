# MFEMNLDiffusionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the non-linear action

!equation
(k(u)\vec\nabla v, \vec\nabla v)_\Omega \,\,\, \forall v \in V

Adds the domain integrator for integrating the bilinear form

!equation
(k(u)\vec\nabla v, \vec\nabla v)_\Omega + (k'(u) v, \vec\nabla u \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $u, v \in H^1$ and $k(u)$ is a scalar non-linear diffusivity coefficient.

The above terms arises from the weak form of the non-linear operator

!equation
- \vec\nabla \cdot \left( k(u) \vec\nabla u \right)

## Example Input File Syntax

!listing mfem/kernels/nldiffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMNLDiffusionKernel

!syntax inputs /Kernels/MFEMNLDiffusionKernel

!syntax children /Kernels/MFEMNLDiffusionKernel

!if-end!

!else
!include mfem/mfem_warning.md
