# MFEMMixedDirectionalDerivativeKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \cdot \vec\nabla u, v)_\Omega \,\,\, \forall v \in V

in 2D or 3D, where $u \in H^1$, $v \in H^1$ or $L^2$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedDirectionalDerivativeKernel/vector_coefficient).

This term arises from the weak form of the operator

!equation
\vec V \cdot \vec\nabla u

`createMBFIntegrator()` returns an [`mfem::MixedDirectionalDerivativeIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedDirectionalDerivativeIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedDirectionalDerivativeKernel

!syntax inputs /Kernels/MFEMMixedDirectionalDerivativeKernel

!syntax children /Kernels/MFEMMixedDirectionalDerivativeKernel

!if-end!

!else
!include mfem/mfem_warning.md
