# MFEMMassKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMMassKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k u, v)_\Omega \,\,\, \forall v \in V

where $u, v \in H^1$ and $k$ is a scalar mass coefficient.

This term arises from the weak form of the mass operator

!equation
k u

!syntax parameters /Kernels/MFEMMassKernel

!syntax inputs /Kernels/MFEMMassKernel

!syntax children /Kernels/MFEMMassKernel

!else
!include mfem/mfem_warning.md
