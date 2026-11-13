# MFEMVectorMassKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u$ and $\vec v$ are vector fields whose components all lie in the same scalar $H^1$ or $L^2$ space. The coefficient $k$ is the scalar set by [!param](/Kernels/MFEMVectorMassKernel/coefficient). Setting [!param](/Kernels/MFEMVectorMassKernel/vector_coefficient) instead replaces $k$ by the diagonal matrix whose diagonal entries are the components of the given vector coefficient; the two parameters cannot be set together.

This term arises from the weak form of the operator

!equation
k \vec u

`createBFIntegrator()` returns an [`mfem::VectorMassIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorMassIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorMassKernel

!syntax inputs /Kernels/MFEMVectorMassKernel

!syntax children /Kernels/MFEMVectorMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
