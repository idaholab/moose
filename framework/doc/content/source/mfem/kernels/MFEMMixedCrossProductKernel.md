# MFEMMixedCrossProductKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $\vec u$ and $\vec v$ are each in $H(\mathrm{curl})$ or $H(\mathrm{div})$ and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossProductKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossProductIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossProductIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossProductKernel

!syntax inputs /Kernels/MFEMMixedCrossProductKernel

!syntax children /Kernels/MFEMMixedCrossProductKernel

!if-end!

!else
!include mfem/mfem_warning.md
