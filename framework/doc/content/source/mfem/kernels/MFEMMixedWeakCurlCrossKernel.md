# MFEMMixedWeakCurlCrossKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $\vec v \in H(\mathrm{curl})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedWeakCurlCrossKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedWeakCurlCrossIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedWeakCurlCrossIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedWeakCurlCrossKernel

!syntax inputs /Kernels/MFEMMixedWeakCurlCrossKernel

!syntax children /Kernels/MFEMMixedWeakCurlCrossKernel

!if-end!

!else
!include mfem/mfem_warning.md
