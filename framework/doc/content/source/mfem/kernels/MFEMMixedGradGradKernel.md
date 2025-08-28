# MFEMMixedGradGradKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMMixedGradGradKernel

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k\vec\nabla u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $u$ and $v$ are both $\in H^1$, and
$k$ is a scalar coefficient.

This term arises from the weak form of the Laplacian operator

!equation
k \vec\nabla u

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedGradGradKernel

!syntax inputs /Kernels/MFEMMixedGradGradKernel

!syntax children /Kernels/MFEMMixedGradGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
