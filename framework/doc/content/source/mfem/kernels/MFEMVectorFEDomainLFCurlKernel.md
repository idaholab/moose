# MFEMVectorFEDomainLFCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
(\vec f, \nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec v \in H(\mathrm{curl})$ is the test variable and $\vec f$ is a vector coefficient.

This term arises from the weak form of the forcing term

!equation
\nabla \times \vec f

and lets the curl of a field that is discontinuous across material interfaces, such as a motional
electromotive force confined to a moving conductor, be applied without projecting it onto
$H(\mathrm{curl})$ first.

## Example Input File Syntax

!listing mfem/kernels/curlcurl_lfcurl.i block=/Kernels

!syntax parameters /Kernels/MFEMVectorFEDomainLFCurlKernel

!syntax inputs /Kernels/MFEMVectorFEDomainLFCurlKernel

!syntax children /Kernels/MFEMVectorFEDomainLFCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
