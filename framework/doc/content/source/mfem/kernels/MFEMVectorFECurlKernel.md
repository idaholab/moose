# MFEMVectorFECurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla \times \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u \in H(\mathrm{curl})$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $k$ is a scalar coefficient. If the trial variable is in $H(\mathrm{div})$ and the test variable in $H(\mathrm{curl})$, the integrator instead assembles $(k \vec u, \vec\nabla \times \vec v)_\Omega$. The coefficient $k$ is set by [!param](/Kernels/MFEMVectorFECurlKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \vec\nabla \times \vec u

`createMBFIntegrator()` returns an [`mfem::VectorFECurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorFECurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorFECurlKernel

!syntax inputs /Kernels/MFEMVectorFECurlKernel

!syntax children /Kernels/MFEMVectorFECurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
