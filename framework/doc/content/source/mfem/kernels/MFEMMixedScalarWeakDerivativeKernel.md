# MFEMMixedScalarWeakDerivativeKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
-\left(k u, \frac{\partial v}{\partial x}\right)_\Omega \,\,\, \forall v \in V

in 1D, where $u \in H^1$ or $L^2$, $v \in H^1$, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMMixedScalarWeakDerivativeKernel/coefficient).

This term arises from the weak form of the operator

!equation
\frac{\partial}{\partial x}\left(k u\right)

`createMBFIntegrator()` returns an [`mfem::MixedScalarWeakDerivativeIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarWeakDerivativeIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarWeakDerivativeKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakDerivativeKernel

!syntax children /Kernels/MFEMMixedScalarWeakDerivativeKernel

!if-end!

!else
!include mfem/mfem_warning.md
