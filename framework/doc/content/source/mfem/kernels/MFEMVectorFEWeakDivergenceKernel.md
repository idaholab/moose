# MFEMVectorFEWeakDivergenceKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMVectorFEWeakDivergenceKernel

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-(k \vec u, \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $\vec u \in H(\mathrm{curl})$ or $H(\mathrm{div})$, $v \in H^1$, and $k$ is a scalar
coefficient.

This term arises from the weak form of the divergence operator

!equation
\vec\nabla \cdot \left(k \vec u\right)

!syntax parameters /Kernels/MFEMVectorFEWeakDivergenceKernel

!syntax inputs /Kernels/MFEMVectorFEWeakDivergenceKernel

!syntax children /Kernels/MFEMVectorFEWeakDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
