# MFEMMixedScalarWeakCurlKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k u, \vec\nabla \times \vec v)_\Omega \,\,\, \forall \vec v \in V

where $u \in H^1$, $\vec v \in H(\mathrm{curl})$ and is 2D, and
$k$ is a scalar coefficient.

This term arises from the weak form of the curl operator

!equation
\vec\nabla \times (k u \hat z)

## Input File Syntax

!syntax parameters /Kernels/MFEMMixedScalarWeakCurlKernel

!syntax inputs /Kernels/MFEMMixedScalarWeakCurlKernel

!syntax children /Kernels/MFEMMixedScalarWeakCurlKernel

!if-end!

!else
!include mfem/mfem_warning.md
