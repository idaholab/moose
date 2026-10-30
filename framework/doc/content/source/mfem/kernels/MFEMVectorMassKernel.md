# MFEMVectorMassKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the bilinear form

!equation
(k \vec u, \vec v)_\Omega \,\,\, \forall \vec v \in V

where $\vec u$ and $\vec v$ belong to vector-valued $H^1$ or $L^2$ spaces, whose components are
each discretized with a scalar basis, and $k$ is a scalar coefficient. If `trial_variable` is set
to a variable other than the test variable, the term is added as a mixed bilinear form coupling the
two. For $H(\mathrm{curl})$ or $H(\mathrm{div})$ variables, use
[MFEMVectorFEMassKernel.md] or [MFEMMixedVectorMassKernel.md] instead.

This term arises from the weak form of the mass operator

!equation
k \vec u

## Example Input File Syntax

!listing mfem/kernels/elastodynamics.i block=/Kernels

!syntax parameters /Kernels/MFEMVectorMassKernel

!syntax inputs /Kernels/MFEMVectorMassKernel

!syntax children /Kernels/MFEMVectorMassKernel

!if-end!

!else
!include mfem/mfem_warning.md
