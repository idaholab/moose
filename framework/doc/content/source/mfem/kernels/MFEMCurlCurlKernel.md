# MFEMCurlCurlKernel

!if! function=hasCapability('mfem')

## Summary

!syntax description /Kernels/MFEMCurlCurlKernel

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k\vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ and $k$ is a scalar coefficient.

This term arises from the weak form of the curl curl operator

!equation
-\vec\nabla \times \left(k \vec\nabla \times \vec u\right)

## Example Input File Syntax

!listing mfem/kernels/curlcurl.i

!syntax parameters /Kernels/MFEMCurlCurlKernel

!syntax inputs /Kernels/MFEMCurlCurlKernel

!syntax children /Kernels/MFEMCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
