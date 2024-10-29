# MFEMVectorFEDomainLFKernel

!syntax description /Kernels/MFEMVectorFEDomainLFKernel

## Overview

Adds the domain integrator for integrating the linear form

!equation
(\vec f, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec v \in H(\mathrm{curl})$ or $H(\mathrm{div})$.

This term arises from the weak form of the forcing term

!equation
\vec f

## Example Input File Syntax

!listing kernels/curlcurl.i

!syntax parameters /Kernels/MFEMVectorFEDomainLFKernel

!syntax inputs /Kernels/MFEMVectorFEDomainLFKernel

!syntax children /Kernels/MFEMVectorFEDomainLFKernel
