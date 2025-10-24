# MFEMDomainLFGradKernel

!if! function=hasCapability('mfem')

!syntax description /Kernels/MFEMDomainLFGradKernel

## Overview

Adds the domain integrator for integrating the linear form

!equation
(\vec{f}, \nabla v)_\Omega \,\,\, \forall v \in V

where $v \in H^1$ is the test variable and $\vec{f}$ is a
vector forcing coefficient.

This term arises from the weak form of the forcing term

!equation
-\nabla \cdot \vec{f}

## Example Input File Syntax

!listing mfem/kernels/nldiffusion.i block=/Kernels

!syntax parameters /Kernels/MFEMDomainLFGradKernel

!syntax inputs /Kernels/MFEMDomainLFGradKernel

!syntax children /Kernels/MFEMDomainLFGradKernel

!if-end!

!else
!include mfem/mfem_warning.md