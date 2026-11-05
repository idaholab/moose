# MFEMMixedDotProductKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \cdot \vec u, v)_\Omega \,\,\, \forall v \in V

in 2D or 3D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $v \in H^1$ or $L^2$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedDotProductKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedDotProductIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedDotProductIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedDotProductKernel

!syntax inputs /Kernels/MFEMMixedDotProductKernel

!syntax children /Kernels/MFEMMixedDotProductKernel

!if-end!

!else
!include mfem/mfem_warning.md
