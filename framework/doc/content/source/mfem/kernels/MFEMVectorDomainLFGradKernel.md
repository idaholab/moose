# MFEMVectorDomainLFGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
\sum_{i} (\vec f_i, \vec\nabla v_i)_\Omega \,\,\, \forall \vec v \in V

where $\vec v = (v_1, \dots, v_n)$ has all components $v_i$ in the same $H^1$ space, and $\vec f = (f_{1x}, f_{1y}, f_{1z}, \dots, f_{nx}, f_{ny}, f_{nz})$ is a vector coefficient holding one vector $\vec f_i$ of mesh dimension per component of $\vec v$. The coefficient $\vec f$ is set by [!param](/Kernels/MFEMVectorDomainLFGradKernel/vector_coefficient).

`createLFIntegrator()` returns an [`mfem::VectorDomainLFGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorDomainLFGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorDomainLFGradKernel

!syntax inputs /Kernels/MFEMVectorDomainLFGradKernel

!syntax children /Kernels/MFEMVectorDomainLFGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
