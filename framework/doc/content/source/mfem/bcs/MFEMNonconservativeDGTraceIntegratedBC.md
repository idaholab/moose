# MFEMNonconservativeDGTraceIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary face integrator for integrating the DG convection flux

!equation
-\alpha \langle \rho_u (\vec Q \cdot \hat n) \{v\}, [u] \rangle
+ \beta \langle \rho_u |\vec Q \cdot \hat n| [v], [u] \rangle \,\,\, \forall v \in V

on the boundary faces of the mesh, where $u$ and $v$ are in the same scalar $L^2$ space, $\hat n$
is the outward unit normal, and both $u$ and $v$ are taken to be zero outside the domain. The
velocity $\vec Q$, the scalar $\rho$ and the weights $\alpha$ and $\beta$ are set by
[!param](/BCs/MFEMNonconservativeDGTraceIntegratedBC/vector_coefficient),
[!param](/BCs/MFEMNonconservativeDGTraceIntegratedBC/coefficient),
[!param](/BCs/MFEMNonconservativeDGTraceIntegratedBC/alpha) and
[!param](/BCs/MFEMNonconservativeDGTraceIntegratedBC/beta), with the same meaning and defaults
as in [MFEMNonconservativeDGTraceKernel](MFEMNonconservativeDGTraceKernel.md). With the defaults
$\alpha = 1$ and $\beta = 1/2$ this term is $-(\vec Q \cdot \hat n) u$ where
$\vec Q \cdot \hat n < 0$ and vanishes on outflow.

A nonzero inflow value is imposed by adding
[MFEMBoundaryFlowIntegratedBC](MFEMBoundaryFlowIntegratedBC.md) on the same boundary.

`createBFIntegrator()` returns an [`mfem::NonconservativeDGTraceIntegrator`](https://docs.mfem.org/html/classmfem_1_1NonconservativeDGTraceIntegrator.html),
which is added to the bilinear form with `AddBdrFaceIntegrator()`.

## Example Input File Syntax

!syntax parameters /BCs/MFEMNonconservativeDGTraceIntegratedBC

!syntax inputs /BCs/MFEMNonconservativeDGTraceIntegratedBC

!syntax children /BCs/MFEMNonconservativeDGTraceIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
