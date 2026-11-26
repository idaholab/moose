# MFEMMixedScalarDerivativeKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
\left(k \frac{\partial u}{\partial x}, v\right)_\Omega \,\,\, \forall v \in V

in 1D, where $u \in H^1$, $v \in H^1$ or $L^2$, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMMixedScalarDerivativeKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \frac{\partial u}{\partial x}

`createMBFIntegrator()` returns an [`mfem::MixedScalarDerivativeIntegrator`](https://docs.mfem.org/html/classmfem_1_1MixedScalarDerivativeIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarDerivativeKernel

!syntax inputs /Kernels/MFEMMixedScalarDerivativeKernel

!syntax children /Kernels/MFEMMixedScalarDerivativeKernel

!if-end!

!else
!include mfem/mfem_warning.md
