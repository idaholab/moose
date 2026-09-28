# MFEMDGElasticityIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary face integrator for integrating the DG isotropic linear elasticity face terms

!equation
- \langle \sigma(\vec u) \cdot \hat n, \vec v \rangle
+ \alpha \langle \sigma(\vec v) \cdot \hat n, \vec u \rangle
+ \kappa \langle h^{-1} (\lambda + 2 \mu) \vec u, \vec v \rangle \,\,\, \forall \vec v \in V

on the boundary faces of the mesh, where $\vec u$ and $\vec v$ are vector fields whose
components all lie in the same scalar $L^2$ space, $\hat n$ is the outward unit normal, $h$ is
the local mesh size and $\sigma$ is the isotropic stress defined in
[MFEMDGElasticityKernel](MFEMDGElasticityKernel.md). The parameters
[!param](/BCs/MFEMDGElasticityIntegratedBC/lambda), [!param](/BCs/MFEMDGElasticityIntegratedBC/mu),
[!param](/BCs/MFEMDGElasticityIntegratedBC/alpha) and
[!param](/BCs/MFEMDGElasticityIntegratedBC/kappa) have the same meaning and defaults as in that
kernel.

On its own, this term weakly imposes $\vec u = 0$ on the boundary. Nonzero Dirichlet data is
imposed by also adding [MFEMDGElasticityDirichletLFIntegratedBC](MFEMDGElasticityDirichletLFIntegratedBC.md)
with the same parameters on the same boundary.

`createBFIntegrator()` returns an [`mfem::DGElasticityIntegrator`](https://docs.mfem.org/html/classmfem_1_1DGElasticityIntegrator.html),
which is added to the bilinear form with `AddBdrFaceIntegrator()`.

## Example Input File Syntax

!syntax parameters /BCs/MFEMDGElasticityIntegratedBC

!syntax inputs /BCs/MFEMDGElasticityIntegratedBC

!syntax children /BCs/MFEMDGElasticityIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
