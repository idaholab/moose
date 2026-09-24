# MFEMNLCurlCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the non-linear form

!equation
(k(|\vec\nabla \times \vec u|)\vec\nabla \times \vec u, \vec\nabla \times \vec v)_\Omega \,\,\,
\forall \vec v \in V

where $\vec u, \vec v \in H(\mathrm{curl})$ and $k$ is a scalar coefficient that may depend on the
magnitude of the curl of the trial variable $\vec u$.

This term arises from the weak form of the curl curl operator

!equation
\vec\nabla \times \left(k(|\vec\nabla \times \vec u|) \vec\nabla \times \vec u\right)

Partial assembly is now supported for 3D problems and on CPU only. The user must supply
a correct function in the input file that corresponds to $ k'(s) / s $, where $ s = | \vec\nabla \times \vec u | $.
This goes into `dk_dcurlu_over_curlu_coefficient`.

## Example Input File Syntax

!listing mfem/submeshes/nl_hphi_magnetodynamic.i block=/Kernels

!syntax parameters /Kernels/MFEMNLCurlCurlKernel

!syntax inputs /Kernels/MFEMNLCurlCurlKernel

!syntax children /Kernels/MFEMNLCurlCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
