# MFEMMixedScalarWeakCrossProductKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(\vec V \times \hat z u, \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D, where $u \in H^1$ or $L^2$, $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $\vec V$ is a two-component vector coefficient, $\hat z$ is the unit vector normal to the plane, and $\vec V \times \hat z = (V_y, -V_x)$. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedScalarWeakCrossProductKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedScalarWeakCrossProductIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarWeakCrossProductIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarWeakCrossProductKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakCrossProductKernel

!syntax children /Kernels/MFEMMixedScalarWeakCrossProductKernel

!if-end!

!else
!include mfem/mfem_warning.md
