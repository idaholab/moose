# MFEMMixedScalarWeakDivergenceKernel

!if! function=hasCapability('mfem')

!syntax description /Kernels/MFEMMixedScalarWeakDivergenceKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(-\vec{\lambda} u, \nabla v)_\Omega \,\,\, \forall v \in V

where $v \in H^1$ is the test variable and $\vec{\lambda}$ is a
vector forcing coefficient.

This term arises from the weak form of the forcing term

!equation
\nabla \cdot(\vec{\lambda} u)

## Example Input File Syntax

!listing mfem/kernels/nldiffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMMixedScalarWeakDivergenceKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakDivergenceKernel

!syntax children /Kernels/MFEMMixedScalarWeakDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md