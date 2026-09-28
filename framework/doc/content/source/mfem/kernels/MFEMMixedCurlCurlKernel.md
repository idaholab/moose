# MFEMMixedCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u$ and $\vec v$ are both in $H(\mathrm{curl})$. The coefficient $k$ is the scalar set by [!param](/Kernels/MFEMMixedCurlCurlKernel/coefficient). Setting [!param](/Kernels/MFEMMixedCurlCurlKernel/vector_coefficient) instead replaces $k$ by the diagonal matrix whose diagonal entries are the components of the given vector coefficient; the two parameters cannot be set together.

This term arises from the weak form of the operator

!equation
\vec\nabla \times \left(k \vec\nabla \times \vec u\right)

`createMBFIntegrator()` returns an [`mfem::MixedCurlCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCurlCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCurlCurlKernel

!syntax inputs /Kernels/MFEMMixedCurlCurlKernel

!syntax children /Kernels/MFEMMixedCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
