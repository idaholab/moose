# MFEMMixedVectorProductKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D or 3D, where $u \in H^1$ or $L^2$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedVectorProductKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedVectorProductIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedVectorProductIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedVectorProductKernel

!syntax inputs /Kernels/MFEMMixedVectorProductKernel

!syntax children /Kernels/MFEMMixedVectorProductKernel

!if-end!

!else
!include mfem/mfem_warning.md
