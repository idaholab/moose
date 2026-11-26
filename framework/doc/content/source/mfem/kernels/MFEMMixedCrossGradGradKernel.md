# MFEMMixedCrossGradGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

in 2D or 3D, where $u$ and $v$ are both in $H^1$ and $\vec V$ is a vector coefficient with three components. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossGradGradKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossGradGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossGradGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossGradGradKernel

!syntax inputs /Kernels/MFEMMixedCrossGradGradKernel

!syntax children /Kernels/MFEMMixedCrossGradGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
