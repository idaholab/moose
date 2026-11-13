# MFEMMixedCrossGradCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $u \in H^1$, $\vec v \in H(\mathrm{curl})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossGradCurlKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossGradCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossGradCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossGradCurlKernel

!syntax inputs /Kernels/MFEMMixedCrossGradCurlKernel

!syntax children /Kernels/MFEMMixedCrossGradCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
