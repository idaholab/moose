# MFEMVectorFEDivergenceKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMVectorFEDivergenceKernel

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec \nabla \cdot \vec u, v)_\Omega \,\,\, \forall v \in V

where $\vec u \in H(\mathrm{div})$, $v \in H^1$ or $L^2$, and $k$ is a scalar coefficient.

This term arises from the weak form of the divergence operator

!equation
k \vec \nabla \cdot \vec u

## Example Input File Syntax

!listing test/tests/mfem/kernels/darcy.i block=/Kernels

!syntax parameters /Kernels/MFEMVectorFEDivergenceKernel

!syntax inputs /Kernels/MFEMVectorFEDivergenceKernel

!syntax children /Kernels/MFEMVectorFEDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
