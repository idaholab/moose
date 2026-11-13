# MFEMDGElasticityDirichletLFIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary face integrator for integrating the linear form

!equation
\alpha \langle \vec u_D, \sigma(\vec v) \cdot \hat n \rangle
+ \kappa \langle h^{-1} (\lambda + 2 \mu) \vec u_D, \vec v \rangle \,\,\, \forall \vec v \in V

on the boundary faces of the mesh, where $\vec v$ is a vector field whose components all lie in
the same scalar $L^2$ space, $\hat n$ is the outward unit normal, $h$ is the local mesh size,
$\sigma$ is the isotropic stress defined in [MFEMDGElasticityKernel](MFEMDGElasticityKernel.md),
and $\vec u_D$ is the Dirichlet data, set by
[!param](/BCs/MFEMDGElasticityDirichletLFIntegratedBC/vector_coefficient).

Together with [MFEMDGElasticityIntegratedBC](MFEMDGElasticityIntegratedBC.md) on the same
boundary, this term weakly imposes $\vec u = \vec u_D$. The parameters
[!param](/BCs/MFEMDGElasticityDirichletLFIntegratedBC/lambda),
[!param](/BCs/MFEMDGElasticityDirichletLFIntegratedBC/mu),
[!param](/BCs/MFEMDGElasticityDirichletLFIntegratedBC/alpha) and
[!param](/BCs/MFEMDGElasticityDirichletLFIntegratedBC/kappa) have the same meaning and defaults as
in [MFEMDGElasticityKernel](MFEMDGElasticityKernel.md), and must match the values used by
[MFEMDGElasticityIntegratedBC](MFEMDGElasticityIntegratedBC.md).

`createLFIntegrator()` returns an [`mfem::DGElasticityDirichletLFIntegrator`](https://docs.mfem.org/html/classmfem_1_1DGElasticityDirichletLFIntegrator.html),
which is added to the linear form with `AddBdrFaceIntegrator()`.

## Example Input File Syntax

!syntax parameters /BCs/MFEMDGElasticityDirichletLFIntegratedBC

!syntax inputs /BCs/MFEMDGElasticityDirichletLFIntegratedBC

!syntax children /BCs/MFEMDGElasticityDirichletLFIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
