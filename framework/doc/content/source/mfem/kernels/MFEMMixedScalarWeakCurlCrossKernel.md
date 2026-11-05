# MFEMMixedScalarWeakCurlCrossKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec u, \nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $\vec v \in H(\mathrm{curl})$, $\vec V$ is a two-component vector coefficient, and both the cross product $\vec V \times \vec u = V_x u_y - V_y u_x$ and the curl $\nabla \times \vec v$ are scalars. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedScalarWeakCurlCrossKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedScalarWeakCurlCrossIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarWeakCurlCrossIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarWeakCurlCrossKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakCurlCrossKernel

!syntax children /Kernels/MFEMMixedScalarWeakCurlCrossKernel

!if-end!

!else
!include mfem/mfem_warning.md
