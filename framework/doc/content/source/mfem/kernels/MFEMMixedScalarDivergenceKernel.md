# MFEMMixedScalarDivergenceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla \cdot \vec u, v)_\Omega \,\,\, \forall v \in V

where $\vec u \in H(\mathrm{div})$, $v \in H^1$ or $L^2$, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMMixedScalarDivergenceKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \vec\nabla \cdot \vec u

`createMBFIntegrator()` returns an [`mfem::MixedScalarDivergenceIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarDivergenceIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarDivergenceKernel

!syntax inputs /Kernels/MFEMMixedScalarDivergenceKernel

!syntax children /Kernels/MFEMMixedScalarDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
