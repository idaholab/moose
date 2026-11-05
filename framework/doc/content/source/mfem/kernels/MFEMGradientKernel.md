# MFEMGradientKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec\nabla u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $u \in H^1$, $\vec v$ is a vector field whose components all lie in the same scalar $H^1$ or $L^2$ space, one component per mesh dimension, and $k$ is a scalar coefficient. The coefficient $k$ is set by [!param](/Kernels/MFEMGradientKernel/coefficient).

This term arises from the weak form of the operator

!equation
k \vec\nabla u

`createMBFIntegrator()` returns an [`mfem::GradientIntegrator`](https://docs.mfem.org/html/classmfem_1_1GradientIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMGradientKernel

!syntax inputs /Kernels/MFEMGradientKernel

!syntax children /Kernels/MFEMGradientKernel

!if-end!

!else
!include mfem/mfem_warning.md
