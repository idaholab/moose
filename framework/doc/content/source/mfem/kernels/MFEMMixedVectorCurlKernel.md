# MFEMMixedVectorCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla \times \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u \in H(\mathrm{curl})$ and $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$. The coefficient $k$ is the scalar set by [!param](/Kernels/MFEMMixedVectorCurlKernel/coefficient). Setting [!param](/Kernels/MFEMMixedVectorCurlKernel/vector_coefficient) instead replaces $k$ by the diagonal matrix whose diagonal entries are the components of the given vector coefficient; the two parameters cannot be set together.

This term arises from the weak form of the operator

!equation
k \vec\nabla \times \vec u

`createMBFIntegrator()` returns an [`mfem::MixedVectorCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedVectorCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedVectorCurlKernel

!syntax inputs /Kernels/MFEMMixedVectorCurlKernel

!syntax children /Kernels/MFEMMixedVectorCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
