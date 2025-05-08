# MFEMVectorDomainLFKernel

!if! function=hasCapability('mfem')

!syntax description /Kernels/MFEMVectorDomainLFKernel

## Overview

Adds the domain integrator for integrating the linear form

!equation
(\vec f, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec v \in \vec H^1$ is the test variable and $\vec f$ is a
vector forcing coefficient.

This term arises from the weak form of the forcing term

!equation
\vec f

## Example Input File Syntax

!listing mfem/kernels/gravity.i block=Kernels

!syntax parameters /Kernels/MFEMVectorDomainLFKernel

!syntax inputs /Kernels/MFEMVectorDomainLFKernel

!syntax children /Kernels/MFEMVectorDomainLFKernel

!if-end!

!else
!include mfem/mfem_warning.md
