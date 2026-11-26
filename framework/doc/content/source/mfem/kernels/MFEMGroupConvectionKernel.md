# MFEMGroupConvectionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(\vec Q \cdot \vec\nabla u, v)_\Omega \,\,\, \forall v \in V

where $u$ and $v$ are in the same nodal scalar $H^1$ space and $\vec Q$ is a vector (velocity) coefficient. The term is discretized with the group finite element formulation, in which $\vec Q$ is evaluated at the nodes of the test functions rather than at the quadrature points. The coefficient $\vec Q$ is set by [!param](/Kernels/MFEMGroupConvectionKernel/vector_coefficient).

This term arises from the weak form of the operator

!equation
\vec Q \cdot \vec\nabla u

`createBFIntegrator()` returns an [`mfem::GroupConvectionIntegrator`](https://docs.mfem.org/html/classmfem_1_1GroupConvectionIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMGroupConvectionKernel

!syntax inputs /Kernels/MFEMGroupConvectionKernel

!syntax children /Kernels/MFEMGroupConvectionKernel

!if-end!

!else
!include mfem/mfem_warning.md
