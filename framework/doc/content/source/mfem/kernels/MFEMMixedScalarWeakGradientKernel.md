# MFEMMixedScalarWeakGradientKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(k u, \vec\nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V

where $u \in H^1$ or $L^2$, $\vec v \in H(\mathrm{div})$, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMMixedScalarWeakGradientKernel/coefficient).

This term arises from the weak form of the operator

!equation
\vec\nabla \left(k u\right)

`createMBFIntegrator()` returns an [`mfem::MixedScalarWeakGradientIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarWeakGradientIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarWeakGradientKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakGradientKernel

!syntax children /Kernels/MFEMMixedScalarWeakGradientKernel

!if-end!

!else
!include mfem/mfem_warning.md
