# MFEMVectorFEDomainLFCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
(\vec f, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec v \in H(\mathrm{curl})$ and $\vec f$ is a vector coefficient. The coefficient $\vec f$ is set by [!param](/Kernels/MFEMVectorFEDomainLFCurlKernel/vector_coefficient).

`createLFIntegrator()` returns an [`mfem::VectorFEDomainLFCurlIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorFEDomainLFCurlIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorFEDomainLFCurlKernel

!syntax inputs /Kernels/MFEMVectorFEDomainLFCurlKernel

!syntax children /Kernels/MFEMVectorFEDomainLFCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
