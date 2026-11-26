# MFEMDGTraceIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary face integrator for integrating the DG convection flux

!equation
\alpha \langle \rho_u (\vec Q \cdot \hat n) \{u\}, [v] \rangle
+ \beta \langle \rho_u |\vec Q \cdot \hat n| [u], [v] \rangle \,\,\, \forall v \in V

on the boundary faces of the mesh, where $u$ and $v$ are in the same scalar $L^2$ space and
$\hat n$ is the outward unit normal. On a boundary face the variable is taken to be zero outside
the domain, so $\{u\} = u/2$ and $[u] = u$. The velocity $\vec Q$, the scalar $\rho$ and the
weights $\alpha$ and $\beta$ are set by
[!param](/BCs/MFEMDGTraceIntegratedBC/vector_coefficient),
[!param](/BCs/MFEMDGTraceIntegratedBC/coefficient),
[!param](/BCs/MFEMDGTraceIntegratedBC/alpha) and [!param](/BCs/MFEMDGTraceIntegratedBC/beta),
with the same meaning and defaults as in [MFEMDGTraceKernel](MFEMDGTraceKernel.md). With the
defaults $\alpha = 1$ and $\beta = 1/2$ this term is the outflow flux $(\vec Q \cdot \hat n) u$
where $\vec Q \cdot \hat n > 0$ and vanishes on inflow.

A nonzero inflow value is imposed by adding
[MFEMBoundaryFlowIntegratedBC](MFEMBoundaryFlowIntegratedBC.md) on the same boundary.

`createBFIntegrator()` returns an [`mfem::DGTraceIntegrator`](https://docs.mfem.org/html/classmfem_1_1DGTraceIntegrator.html),
which is added to the bilinear form with `AddBdrFaceIntegrator()`.

## Example Input File Syntax

!syntax parameters /BCs/MFEMDGTraceIntegratedBC

!syntax inputs /BCs/MFEMDGTraceIntegratedBC

!syntax children /BCs/MFEMDGTraceIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
