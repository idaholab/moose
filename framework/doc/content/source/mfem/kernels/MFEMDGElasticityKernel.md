# MFEMDGElasticityKernel

!if! function=hasCapability('mfem')

## Overview

Adds the interior face integrator for the DG face terms of isotropic linear elasticity,

!equation
- \langle \{ \sigma(\vec u) \cdot \hat n \}, [\vec v] \rangle
+ \alpha \langle \{ \sigma(\vec v) \cdot \hat n \}, [\vec u] \rangle
+ \kappa \langle h^{-1} \{ \lambda + 2 \mu \} [\vec u], [\vec v] \rangle \,\,\, \forall \vec v \in V

over the interior faces of the mesh, where $\vec u$ and $\vec v$ are vector fields whose
components all lie in the same scalar $L^2$ space, $\{\cdot\}$ and $[\cdot]$ are the face average
and jump, $h$ is the local mesh size, and

!equation
\sigma(\vec u) = \lambda (\vec\nabla \cdot \vec u) I + \mu (\vec\nabla \vec u + \vec\nabla \vec u^T)

is the isotropic stress. The Lamé parameters $\lambda$ and $\mu$ are set by
[!param](/Kernels/MFEMDGElasticityKernel/lambda) and [!param](/Kernels/MFEMDGElasticityKernel/mu).
The parameter $\alpha$, set by [!param](/Kernels/MFEMDGElasticityKernel/alpha), selects the DG
method: $\alpha = -1$ (the default) gives the symmetric interior penalty method (SIPG),
$\alpha = 0$ the incomplete method (IIPG) and $\alpha = 1$ the non-symmetric method (NIPG). The
penalty parameter $\kappa$ is set by [!param](/Kernels/MFEMDGElasticityKernel/kappa) and defaults
to $(o+1)^2$, where $o$ is the order of the finite element space of the variable.

This term is used alongside [MFEMLinearElasticityKernel](MFEMLinearElasticityKernel.md) with the
same Lamé parameters. Dirichlet conditions are imposed weakly with
[MFEMDGElasticityIntegratedBC](MFEMDGElasticityIntegratedBC.md) and
[MFEMDGElasticityDirichletLFIntegratedBC](MFEMDGElasticityDirichletLFIntegratedBC.md).

`createBFIntegrator()` returns an [`mfem::DGElasticityIntegrator`](https://docs.mfem.org/html/classmfem_1_1DGElasticityIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMDGElasticityKernel

!syntax inputs /Kernels/MFEMDGElasticityKernel

!syntax children /Kernels/MFEMDGElasticityKernel

!if-end!

!else
!include mfem/mfem_warning.md
