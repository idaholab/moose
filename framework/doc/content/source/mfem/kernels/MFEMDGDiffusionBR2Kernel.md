# MFEMDGDiffusionBR2Kernel

!if! function=hasCapability('mfem')

## Overview

Adds the interior face integrator for the stabilization term of the second method of Bassi and
Rebay (BR2) for DG diffusion,

!equation
\sum_e \eta (r_e([u]), r_e([v])) \,\,\, \forall v \in V

where the sum runs over the interior faces $e$ of the mesh, $u$ and $v$ are in the same scalar
$L^2$ space, $[u]$ is the jump of $u$ across face $e$, and $r_e$ is the lifting operator of face
$e$, weighted by the diffusion coefficient $k$ set by
[!param](/Kernels/MFEMDGDiffusionBR2Kernel/coefficient). The penalty parameter $\eta$ is set by
[!param](/Kernels/MFEMDGDiffusionBR2Kernel/eta); $\eta = 1$ gives a stable discretization.

This term is used alongside [MFEMDiffusionKernel](MFEMDiffusionKernel.md) and
[MFEMDGDiffusionKernel](MFEMDGDiffusionKernel.md). The matching boundary face term is added by
[MFEMDGDiffusionBR2IntegratedBC](MFEMDGDiffusionBR2IntegratedBC.md). MFEM requires the variable to be in a DG space
and does not support partial assembly for this integrator.

`createBFIntegrator()` returns an [`mfem::DGDiffusionBR2Integrator`](https://docs.mfem.org/html/classmfem_1_1DGDiffusionBR2Integrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMDGDiffusionBR2Kernel

!syntax inputs /Kernels/MFEMDGDiffusionBR2Kernel

!syntax children /Kernels/MFEMDGDiffusionBR2Kernel

!if-end!

!else
!include mfem/mfem_warning.md
