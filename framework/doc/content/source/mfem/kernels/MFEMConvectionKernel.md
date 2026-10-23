# MFEMConvectionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(\vec Q \cdot \vec\nabla u, v)_\Omega \,\,\, \forall v \in V

where $u$ and $v$ are in the same scalar $H^1$ or $L^2$ space and $\vec Q$ is a vector (velocity) coefficient. The coefficient $\vec Q$ is set by [!param](/Kernels/MFEMConvectionKernel/vector_coefficient).

This term arises from the weak form of the operator

!equation
\vec Q \cdot \vec\nabla u

`createBFIntegrator()` returns an [`mfem::ConvectionIntegrator`](https://docs.mfem.org/html/classmfem_1_1ConvectionIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMConvectionKernel

!syntax inputs /Kernels/MFEMConvectionKernel

!syntax children /Kernels/MFEMConvectionKernel

!if-end!

!else
!include mfem/mfem_warning.md
