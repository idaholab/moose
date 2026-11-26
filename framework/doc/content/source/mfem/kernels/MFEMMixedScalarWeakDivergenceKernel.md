# MFEMMixedScalarWeakDivergenceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(\vec V u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

in 2D or 3D, where $u \in H^1$ or $L^2$, $v \in H^1$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedScalarWeakDivergenceKernel/vector_coefficient).

This term arises from the weak form of the operator

!equation
\vec\nabla \cdot \left(\vec V u\right)

`createMBFIntegrator()` returns an [`mfem::MixedScalarWeakDivergenceIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarWeakDivergenceIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarWeakDivergenceKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakDivergenceKernel

!syntax children /Kernels/MFEMMixedScalarWeakDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
