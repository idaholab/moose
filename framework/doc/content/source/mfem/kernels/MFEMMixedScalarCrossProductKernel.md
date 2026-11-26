# MFEMMixedScalarCrossProductKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \vec u, v)_\Omega \,\,\, \forall v \in V

in 2D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $v \in H^1$ or $L^2$, $\vec V$ is a two-component vector coefficient, and the cross product $\vec V \times \vec u = V_x u_y - V_y u_x$ is a scalar. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedScalarCrossProductKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedScalarCrossProductIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarCrossProductIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarCrossProductKernel

!syntax inputs /Kernels/MFEMMixedScalarCrossProductKernel

!syntax children /Kernels/MFEMMixedScalarCrossProductKernel

!if-end!

!else
!include mfem/mfem_warning.md
