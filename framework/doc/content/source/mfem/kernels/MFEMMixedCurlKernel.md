# MFEMMixedCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla \times \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $k$ is a scalar coefficient and the variables fall into one of three cases: $\vec u \in H(\mathrm{curl})$ in 3D and $\vec v$ a 3D vector field with components in $H^1$ or $L^2$; $\vec u \in H(\mathrm{curl})$ in 2D and $v$ a scalar field in $H^1$ or $L^2$; or $u$ a scalar $H^1$ field in 2D, whose curl is the rotated gradient $(\partial_y u, -\partial_x u)$, and $\vec v$ a 2D vector field with components in $H^1$ or $L^2$. The coefficient $k$ is set by [!param](/Kernels/MFEMMixedCurlKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \vec\nabla \times \vec u

`createMBFIntegrator()` returns an [`mfem::MixedCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCurlKernel

!syntax inputs /Kernels/MFEMMixedCurlKernel

!syntax children /Kernels/MFEMMixedCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
