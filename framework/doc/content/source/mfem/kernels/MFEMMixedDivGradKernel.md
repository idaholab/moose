# MFEMMixedDivGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(\vec V \vec\nabla \cdot \vec u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

in 2D or 3D, where $\vec u \in H(\mathrm{div})$, $v \in H^1$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedDivGradKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedDivGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedDivGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedDivGradKernel

!syntax inputs /Kernels/MFEMMixedDivGradKernel

!syntax children /Kernels/MFEMMixedDivGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
