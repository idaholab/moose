# MFEMDGTraceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the interior face integrator for integrating the DG convection flux

!equation
\alpha \langle \rho_u (\vec Q \cdot \hat n) \{u\}, [v] \rangle
+ \beta \langle \rho_u |\vec Q \cdot \hat n| [u], [v] \rangle \,\,\, \forall v \in V

over the interior faces of the mesh, where $u$ and $v$ are in the same scalar $L^2$ space,
$\{u\} = (u_1 + u_2)/2$ and $[u] = u_1 - u_2$ are the average and jump of $u$ across a face
between elements 1 and 2, and $\hat n$ is the face normal pointing from element 1 into element 2.
The velocity $\vec Q$ is set by [!param](/Kernels/MFEMDGTraceKernel/vector_coefficient) and is
assumed continuous across faces. The scalar coefficient $\rho$ is set by
[!param](/Kernels/MFEMDGTraceKernel/coefficient) and may be discontinuous; $\rho_u$ is its value
in the element that $\vec Q$ points into across the face. The weights $\alpha$ and
$\beta$ are set by [!param](/Kernels/MFEMDGTraceKernel/alpha) and
[!param](/Kernels/MFEMDGTraceKernel/beta); if [!param](/Kernels/MFEMDGTraceKernel/beta) is not
set it defaults to $\alpha/2$.

Together with [MFEMConservativeConvectionKernel](MFEMConservativeConvectionKernel.md), the
defaults $\alpha = 1$ and $\beta = 1/2$ give the upwind DG discretization of the operator

!equation
\vec\nabla \cdot (\vec Q u)

on the left hand side of the equation. The matching boundary face term is added by
[MFEMDGTraceIntegratedBC](MFEMDGTraceIntegratedBC.md) with the same parameters, and inflow values
by [MFEMBoundaryFlowIntegratedBC](MFEMBoundaryFlowIntegratedBC.md).

`createBFIntegrator()` returns an [`mfem::DGTraceIntegrator`](https://docs.mfem.org/html/classmfem_1_1DGTraceIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMDGTraceKernel

!syntax inputs /Kernels/MFEMDGTraceKernel

!syntax children /Kernels/MFEMDGTraceKernel

!if-end!

!else
!include mfem/mfem_warning.md
