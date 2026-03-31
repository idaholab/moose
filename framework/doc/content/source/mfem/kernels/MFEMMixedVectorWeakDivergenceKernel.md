# MFEMMixedVectorWeakDivergenceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(-k \vec u, \vec\nabla v)_\Omega \,\,\, \forall \vec v \in V

where $u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $v \in H^1$, and
$k$ is a scalar coefficient.

This term arises from the weak form of the scalar curl operator

!equation
\vec\nabla \cdot (k \vec u)

## Example Input File Syntax

!listing mfem/complex/mixed_sesquilinear.i block=Kernels

!syntax parameters /Kernels/MFEMMixedVectorWeakDivergenceKernel

!syntax inputs /Kernels/MFEMMixedVectorWeakDivergenceKernel

!syntax children /Kernels/MFEMMixedVectorWeakDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
