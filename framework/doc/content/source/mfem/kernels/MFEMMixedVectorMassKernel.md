# MFEMMixedVectorMassKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMMixedVectorMassKernel

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u$ and $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $k$ is a scalar
coefficient.

This term arises from the weak form of the mass operator

!equation
k u

!syntax parameters /Kernels/MFEMMixedVectorMassKernel

!syntax inputs /Kernels/MFEMMixedVectorMassKernel

!syntax children /Kernels/MFEMMixedVectorMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
