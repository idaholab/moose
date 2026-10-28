# MFEMDomainLFGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
(\vec f, \vec\nabla v)_\Omega \,\,\, \forall v \in V

where $v \in H^1$ and $\vec f$ is a vector coefficient. The coefficient $\vec f$ is set by [!param](/Kernels/MFEMDomainLFGradKernel/vector_coefficient).

`createLFIntegrator()` returns an [`mfem::DomainLFGradIntegrator`](https://docs.mfem.org/html/classmfem_1_1DomainLFGradIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMDomainLFGradKernel

!syntax inputs /Kernels/MFEMDomainLFGradKernel

!syntax children /Kernels/MFEMDomainLFGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
