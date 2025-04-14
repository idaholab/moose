# MFEMMixedScalarCurlKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMMixedScalarCurlKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k\vec\nabla \times \vec u, v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u \in H(\mathrm{curl})$ and is 2D, $v \in H^1$, and
$k$ is a scalar coefficient.

This term arises from the weak form of the scalar curl operator

!equation
k \vec\nabla \times \vec u

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarCurlKernel

!syntax inputs /Kernels/MFEMMixedScalarCurlKernel

!syntax children /Kernels/MFEMMixedScalarCurlKernel

!else
!include mfem/mfem_warning.md
