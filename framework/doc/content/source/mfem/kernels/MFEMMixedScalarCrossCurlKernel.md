# MFEMMixedScalarCrossCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla \times \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D, where $\vec u \in H(\mathrm{curl})$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $\vec V$ is a two-component vector coefficient, and the curl $\vec\nabla \times \vec u = (\partial_x u_y - \partial_y u_x) \hat z$ is normal to the plane, so the cross product lies in the plane. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedScalarCrossCurlKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedScalarCrossCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarCrossCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarCrossCurlKernel

!syntax inputs /Kernels/MFEMMixedScalarCrossCurlKernel

!syntax children /Kernels/MFEMMixedScalarCrossCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
