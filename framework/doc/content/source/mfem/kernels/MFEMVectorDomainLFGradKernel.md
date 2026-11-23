# MFEMVectorDomainLFGradKernel

!if! function=hasCapability('mfem')

## Overview

Adds the domain integrator for integrating the linear form

!equation
(F, \vec \nabla \vec v)_\Omega = (F_{ij}, \partial_j v_i)_\Omega \,\,\, \forall \vec v \in V

where $\vec v \in (H^1)^d$ is a vector test function whose components share a scalar finite element
space, and $F$ is a $d \times d$ tensor given by `matrix_coefficient`.

If `matrix_coefficient` is not set, $F = f \mathbf{I}$ for the scalar `coefficient` $f$, and the
linear form reduces to

!equation
(f, \vec \nabla \cdot \vec v)_\Omega \,\,\, \forall \vec v \in V

This term arises from the weak form of the divergence of a known tensor field, such as a prescribed
stress $-\vec \nabla \cdot F$. In linear thermoelasticity, the isotropic form supplies the
contribution of the stress-free reference temperature to the thermal stress, as illustrated in the
example below.

## Example Input File Syntax

!listing test/tests/mfem/kernels/thermal_expansion.i block=/Kernels

!listing test/tests/mfem/kernels/vector_domain_lf_grad.i block=/Kernels

!syntax parameters /Kernels/MFEMVectorDomainLFGradKernel

!syntax inputs /Kernels/MFEMVectorDomainLFGradKernel

!syntax children /Kernels/MFEMVectorDomainLFGradKernel

!if-end!

!else
!include mfem/mfem_warning.md
