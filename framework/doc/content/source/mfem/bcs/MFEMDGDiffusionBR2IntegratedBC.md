# MFEMDGDiffusionBR2IntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary face integrator for the stabilization term of the second method of Bassi and
Rebay (BR2) for DG diffusion,

!equation
\sum_e \eta (r_e([u]), r_e([v])) \,\,\, \forall v \in V

where the sum runs over the boundary faces $e$ of the mesh, $u$ and $v$ are in the same scalar
$L^2$ space and are taken to be zero outside the domain, and $r_e$ is the lifting operator of
face $e$, weighted by the diffusion coefficient $k$ set by
[!param](/BCs/MFEMDGDiffusionBR2IntegratedBC/coefficient). The penalty parameter $\eta$ is set by
[!param](/BCs/MFEMDGDiffusionBR2IntegratedBC/eta).

This is the boundary counterpart of [MFEMDGDiffusionBR2Kernel](MFEMDGDiffusionBR2Kernel.md).
MFEM provides no matching right hand side term, so it is consistent with homogeneous Dirichlet
data only. MFEM requires the variable to be in a DG space and does not support partial assembly
for this integrator.

`createBFIntegrator()` returns an [`mfem::DGDiffusionBR2Integrator`](https://docs.mfem.org/html/classmfem_1_1DGDiffusionBR2Integrator.html),
which is added to the bilinear form with `AddBdrFaceIntegrator()`.

## Example Input File Syntax

!syntax parameters /BCs/MFEMDGDiffusionBR2IntegratedBC

!syntax inputs /BCs/MFEMDGDiffusionBR2IntegratedBC

!syntax children /BCs/MFEMDGDiffusionBR2IntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
