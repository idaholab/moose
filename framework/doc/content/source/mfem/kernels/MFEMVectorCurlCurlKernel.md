# MFEMVectorCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u$ and $\vec v$ are vector fields whose components all lie in the same scalar $H^1$ space, one component per mesh dimension, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMVectorCurlCurlKernel/coefficient).

This term arises from the weak form of the operator

!equation
\vec\nabla \times \left(k \vec\nabla \times \vec u\right)

`createBFIntegrator()` returns an [`mfem::VectorCurlCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorCurlCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorCurlCurlKernel

!syntax inputs /Kernels/MFEMVectorCurlCurlKernel

!syntax children /Kernels/MFEMVectorCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
