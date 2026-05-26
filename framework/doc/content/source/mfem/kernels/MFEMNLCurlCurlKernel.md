# MFEMNLCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the non-linear form

!equation
(k(|\vec\nabla \times \vec u|)\vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\,
\forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ and $k$ is a scalar coefficient that may depend on the
magnitude of the curl of the trial variable $u$.

This term arises from the weak form of the curl curl operator

!equation
\vec\nabla \times \left(k(|\vec\nabla \times \vec u|) \vec\nabla \times \vec u\right)

## Example Input File Syntax

!listing mfem/submeshes/nl_hphi_magnetodynamic.i block=/Kernels

!syntax parameters /Kernels/MFEMNLCurlCurlKernel

!syntax inputs /Kernels/MFEMNLCurlCurlKernel

!syntax children /Kernels/MFEMNLCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
