# MFEMMixedVectorWeakCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$ and $\vec v \in H(\mathrm{curl})$. The coefficient $k$ is the scalar set by [!param](/Kernels/MFEMMixedVectorWeakCurlKernel/coefficient). Setting [!param](/Kernels/MFEMMixedVectorWeakCurlKernel/vector_coefficient) instead replaces $k$ by the diagonal matrix whose diagonal entries are the components of the given vector coefficient; the two parameters cannot be set together.

This term arises from the weak form of the operator

!equation
\vec\nabla \times \left(k \vec u\right)

`createMBFIntegrator()` returns an [`mfem::MixedVectorWeakCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedVectorWeakCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedVectorWeakCurlKernel

!syntax inputs /Kernels/MFEMMixedVectorWeakCurlKernel

!syntax children /Kernels/MFEMMixedVectorWeakCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
