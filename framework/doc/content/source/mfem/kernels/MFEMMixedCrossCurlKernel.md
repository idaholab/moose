# MFEMMixedCrossCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla \times \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u \in H(\mathrm{curl})$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossCurlKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossCurlKernel

!syntax inputs /Kernels/MFEMMixedCrossCurlKernel

!syntax children /Kernels/MFEMMixedCrossCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
