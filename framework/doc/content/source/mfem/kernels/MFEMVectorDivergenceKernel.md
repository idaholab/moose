# MFEMVectorDivergenceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla \cdot \vec u, v)_\Omega \,\,\, \forall v \in V

where $\vec u$ is a vector field whose components all lie in the same scalar $H^1$ space, $v$ is in a (possibly different) scalar space, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMVectorDivergenceKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \vec\nabla \cdot \vec u

`createMBFIntegrator()` returns an [`mfem::VectorDivergenceIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorDivergenceIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorDivergenceKernel

!syntax inputs /Kernels/MFEMVectorDivergenceKernel

!syntax children /Kernels/MFEMVectorDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
