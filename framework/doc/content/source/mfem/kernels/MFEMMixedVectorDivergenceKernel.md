# MFEMMixedVectorDivergenceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \vec\nabla \cdot \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D or 3D, where $\vec u \in H(\mathrm{div})$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedVectorDivergenceKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedVectorDivergenceIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedVectorDivergenceIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedVectorDivergenceKernel

!syntax inputs /Kernels/MFEMMixedVectorDivergenceKernel

!syntax children /Kernels/MFEMMixedVectorDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
