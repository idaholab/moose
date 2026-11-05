# MFEMMixedCrossGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 3D, where $u \in H^1$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedCrossGradKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedCrossGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedCrossGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedCrossGradKernel

!syntax inputs /Kernels/MFEMMixedCrossGradKernel

!syntax children /Kernels/MFEMMixedCrossGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
