# MFEMConservativeConvectionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
-(u, \vec Q \cdot \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $u$ and $v$ are in the same scalar $H^1$ or $L^2$ space and $\vec Q$ is a vector (velocity) coefficient. The coefficient $\vec Q$ is set by [!param](/Kernels/MFEMConservativeConvectionKernel/vector_coefficient).

This term arises from the weak form of the operator

!equation
\vec\nabla \cdot (\vec Q u)

`createBFIntegrator()` returns an [`mfem::ConservativeConvectionIntegrator`](https://docs.mfem.org/html/classmfem_1_1ConservativeConvectionIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMConservativeConvectionKernel

!syntax inputs /Kernels/MFEMConservativeConvectionKernel

!syntax children /Kernels/MFEMConservativeConvectionKernel

!if-end!

!else
!include mfem/mfem_warning.md
