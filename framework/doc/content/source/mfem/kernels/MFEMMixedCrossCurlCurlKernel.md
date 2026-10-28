# MFEMMixedCrossCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u$ and $\vec v$ are both in $H(\mathrm{curl})$ and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossCurlCurlKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossCurlCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossCurlCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossCurlCurlKernel

!syntax inputs /Kernels/MFEMMixedCrossCurlCurlKernel

!syntax children /Kernels/MFEMMixedCrossCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
