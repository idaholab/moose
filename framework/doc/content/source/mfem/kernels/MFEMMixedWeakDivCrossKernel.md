# MFEMMixedWeakDivCrossKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(\vec V \times \vec u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

in 3D, where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $v \in H^1$, and $\vec V$ is a vector coefficient. The coefficient $\vec V$ is set by [!param](/Kernels/MFEMMixedWeakDivCrossKernel/vector_coefficient).

`createMBFIntegrator()` returns an [`mfem::MixedWeakDivCrossIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedWeakDivCrossIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedWeakDivCrossKernel

!syntax inputs /Kernels/MFEMMixedWeakDivCrossKernel

!syntax children /Kernels/MFEMMixedWeakDivCrossKernel

!if-end!

!else
!include mfem/mfem_warning.md
