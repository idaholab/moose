# MFEMMixedGradDivKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(\vec V \cdot \vec\nabla u, \vec\nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D or 3D, where $u \in H^1$, $\vec v \in H(\mathrm{div})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedGradDivKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedGradDivIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedGradDivIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedGradDivKernel

!syntax inputs /Kernels/MFEMMixedGradDivKernel

!syntax children /Kernels/MFEMMixedGradDivKernel

!if-end!

!else
!include mfem/mfem_warning.md
