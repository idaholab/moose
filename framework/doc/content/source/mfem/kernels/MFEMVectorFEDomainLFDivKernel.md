# MFEMVectorFEDomainLFDivKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
(f, \vec\nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec v \in H(\mathrm{div})$ and $f$ is a scalar coefficient. The coefficient $f$ is set by [!param](/Kernels/MFEMVectorFEDomainLFDivKernel/coefficient).

`createLFIntegrator()` returns an [`mfem::VectorFEDomainLFDivIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorFEDomainLFDivIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMVectorFEDomainLFDivKernel

!syntax inputs /Kernels/MFEMVectorFEDomainLFDivKernel

!syntax children /Kernels/MFEMVectorFEDomainLFDivKernel

!if-end!

!else
!include mfem/mfem_warning.md
