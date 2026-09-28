# MFEMMixedCrossCurlGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla \times \vec u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

in 3D, where $\vec u \in H(\mathrm{curl})$, $v \in H^1$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossCurlGradKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossCurlGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossCurlGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossCurlGradKernel

!syntax inputs /Kernels/MFEMMixedCrossCurlGradKernel

!syntax children /Kernels/MFEMMixedCrossCurlGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
