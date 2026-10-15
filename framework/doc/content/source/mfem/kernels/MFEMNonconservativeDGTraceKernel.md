# MFEMNonconservativeDGTraceKernel

!if! function=hasCapability('mfem')

## Overview

Adds the interior face integrator for integrating the DG convection flux

!equation
-\alpha \langle \rho_u (\vec Q \cdot \hat n) \{v\}, [u] \rangle
+ \beta \langle \rho_u |\vec Q \cdot \hat n| [v], [u] \rangle \,\,\, \forall v \in V

over the interior faces of the mesh. This is the transpose of the flux added by
[MFEMDGTraceKernel](MFEMDGTraceKernel.md) with $\alpha$ replaced by $-\alpha$, and the notation
and parameters are the same: $u$ and $v$ are in the same scalar $L^2$ space, $\{\cdot\}$ and
$[\cdot]$ are the face average and jump, the velocity $\vec Q$ is set by
[!param](/Kernels/MFEMNonconservativeDGTraceKernel/vector_coefficient), the scalar $\rho_u$ comes
from [!param](/Kernels/MFEMNonconservativeDGTraceKernel/coefficient), and the
weights are set by [!param](/Kernels/MFEMNonconservativeDGTraceKernel/alpha) and
[!param](/Kernels/MFEMNonconservativeDGTraceKernel/beta), the latter defaulting to $\alpha/2$.

Together with [MFEMConvectionKernel](MFEMConvectionKernel.md), the defaults $\alpha = 1$ and
$\beta = 1/2$ give the upwind DG discretization of the operator

!equation
\vec Q \cdot \vec\nabla u

on the left hand side of the equation. The matching boundary face term is added by
[MFEMNonconservativeDGTraceIntegratedBC](MFEMNonconservativeDGTraceIntegratedBC.md) with the same
parameters, and inflow values by [MFEMBoundaryFlowIntegratedBC](MFEMBoundaryFlowIntegratedBC.md).

`createBFIntegrator()` returns an [`mfem::NonconservativeDGTraceIntegrator`](https://docs.mfem.org/html/classmfem_1_1NonconservativeDGTraceIntegrator.html).

## Example Input File Syntax

!syntax parameters /Kernels/MFEMNonconservativeDGTraceKernel

!syntax inputs /Kernels/MFEMNonconservativeDGTraceKernel

!syntax children /Kernels/MFEMNonconservativeDGTraceKernel

!if-end!

!else
!include mfem/mfem_warning.md
