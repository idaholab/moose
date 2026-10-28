# MFEMMixedScalarCrossGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec\nabla u, v)_\Omega \,\,\, \forall v \in V

in 2D, where $u \in H^1$, $v \in H^1$ or $L^2$, $\vec V$ is a two-component vector coefficient, and the cross product $\vec V \times \vec\nabla u = V_x \partial_y u - V_y \partial_x u$ is a scalar. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedScalarCrossGradKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedScalarCrossGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarCrossGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarCrossGradKernel

!syntax inputs /Kernels/MFEMMixedScalarCrossGradKernel

!syntax children /Kernels/MFEMMixedScalarCrossGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
