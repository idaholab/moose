# MFEMVectorDivergenceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the mixed bilinear form

!equation
(k \vec \nabla \cdot \vec u, v)_\Omega \,\,\, \forall v \in V

where $\vec u \in (H^1)^d$ is a vector variable whose components share a scalar finite element
space, $v \in H^1$ or $L^2$, and $k$ is a scalar coefficient.

This term arises from the weak form of the divergence operator

!equation
k \vec \nabla \cdot \vec u

With `transpose = true`, the roles of the test and trial variables are swapped, giving the form
$(k u, \vec \nabla \cdot \vec v)_\Omega$ for a scalar trial variable $u$ and an $(H^1)^d$ vector test
function $\vec v$. This is the form taken by the coupling of a thermal strain to the displacement
in linear thermoelasticity, as illustrated in the example below.

## Example Input File Syntax

!listing test/tests/mfem/kernels/thermal_expansion.i block=/Kernels

!syntax parameters /Kernels/MFEMVectorDivergenceKernel

!syntax inputs /Kernels/MFEMVectorDivergenceKernel

!syntax children /Kernels/MFEMVectorDivergenceKernel

!if-end!

!else
!include mfem/mfem_warning.md
