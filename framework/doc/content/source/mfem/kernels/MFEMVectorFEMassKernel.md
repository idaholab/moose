# MFEMVectorFEMassKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMVectorFEMassKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$, and $k$ is a scalar coefficient.

This term arises from the weak form of the mass operator

!equation
k \vec u

## Example Input File Syntax

!listing mfem/kernels/curlcurl.i

!syntax parameters /Kernels/MFEMVectorFEMassKernel

!syntax inputs /Kernels/MFEMVectorFEMassKernel

!syntax children /Kernels/MFEMVectorFEMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
