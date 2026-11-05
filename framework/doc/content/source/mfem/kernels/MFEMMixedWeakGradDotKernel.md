# MFEMMixedWeakGradDotKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(\vec V \cdot \vec u, \vec\nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V

in 2D or 3D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $\vec v \in H(\mathrm{div})$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedWeakGradDotKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedWeakGradDotIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedWeakGradDotIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedWeakGradDotKernel

!syntax inputs /Kernels/MFEMMixedWeakGradDotKernel

!syntax children /Kernels/MFEMMixedWeakGradDotKernel

!if-end!

!else
!include mfem/mfem_warning.md
