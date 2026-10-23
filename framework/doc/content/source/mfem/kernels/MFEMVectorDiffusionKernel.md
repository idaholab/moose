# MFEMVectorDiffusionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
\sum_i (k \vec\nabla u_i, \vec\nabla v_i)_\Omega \,\,\, \forall \vec v \in V

where $\vec u = (u_1, \dots, u_n)$ and $\vec v = (v_1, \dots, v_n)$ are vector fields whose components all lie in the same scalar $H^1$ or $L^2$ space, one component per mesh dimension. The coefficient $k$ is the scalar set by [!param](/Kernels/MFEMVectorDiffusionKernel/coefficient). Setting [!param](/Kernels/MFEMVectorDiffusionKernel/vector_coefficient) instead replaces $k$ by the diagonal matrix whose diagonal entries are the components of the given vector coefficient; the two parameters cannot be set together.

This term arises from the weak form of the operator

!equation
-\vec\nabla \cdot \left(k \vec\nabla \vec u\right)

`createBFIntegrator()` returns an [`mfem::VectorDiffusionIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorDiffusionIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorDiffusionKernel

!syntax inputs /Kernels/MFEMVectorDiffusionKernel

!syntax children /Kernels/MFEMVectorDiffusionKernel

!if-end!

!else
!include mfem/mfem_warning.md
