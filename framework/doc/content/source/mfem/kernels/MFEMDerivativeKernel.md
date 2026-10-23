# MFEMDerivativeKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \partial_i u, v)_\Omega \,\,\, \forall v \in V

where $u$ and $v$ are scalar fields, $u$ has a gradient (for example $u \in H^1$), and
$\partial_i$ is the partial derivative along the spatial direction $i$ selected by
[!param](/Kernels/MFEMDerivativeKernel/component) (0 for $x$, 1 for $y$, 2 for $z$). The
coefficient $k$ is set by [!param](/Kernels/MFEMDerivativeKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \partial_i u

`createMBFIntegrator()` returns an [`mfem::DerivativeIntegrator`](https://docs.mfem.org/html/classmfem_1_1DerivativeIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMDerivativeKernel

!syntax inputs /Kernels/MFEMDerivativeKernel

!syntax children /Kernels/MFEMDerivativeKernel

!if-end!

!else
!include mfem/mfem_warning.md
