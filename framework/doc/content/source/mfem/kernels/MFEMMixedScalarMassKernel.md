# MFEMMixedScalarMassKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k u, v)_\Omega \,\,\, \forall v \in V

where $u$ and $v$ are each in $H^1$ or $L^2$ and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMMixedScalarMassKernel/coefficient).

This term arises from the weak form of the operator

!equation
k u

`createMBFIntegrator()` returns an [`mfem::MixedScalarMassIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarMassIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarMassKernel

!syntax inputs /Kernels/MFEMMixedScalarMassKernel

!syntax children /Kernels/MFEMMixedScalarMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
