# MFEMConvectionKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(\vec\beta \cdot \vec\nabla u, v)_\Omega \,\,\, \forall v \in V

where $u, v \in H^1$ and $\vec\beta$ is a vector velocity coefficient.

This term arises from the weak form of the convection operator

!equation
\vec\beta \cdot \vec\nabla u

## Example Input File Syntax

!listing mfem/complex/complex_magnetic_eigenproblem.i block=/Kernels

!syntax parameters /Kernels/MFEMConvectionKernel

!syntax inputs /Kernels/MFEMConvectionKernel

!syntax children /Kernels/MFEMConvectionKernel

!if-end!

!else
!include mfem/mfem_warning.md
