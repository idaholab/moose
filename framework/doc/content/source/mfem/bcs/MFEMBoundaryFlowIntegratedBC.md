# MFEMBoundaryFlowIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary face integrator for integrating the linear form

!equation
\frac{\alpha}{2} \langle (\vec Q \cdot \hat n) f, v \rangle
- \beta \langle |\vec Q \cdot \hat n| f, v \rangle \,\,\, \forall v \in V

on the boundary faces of the mesh, where $v$ is in a scalar $L^2$ space, $\hat n$ is the outward
unit normal, $f$ is the value of the variable carried into the domain by the flow, set by
[!param](/BCs/MFEMBoundaryFlowIntegratedBC/coefficient), and $\vec Q$ is the velocity, set by
[!param](/BCs/MFEMBoundaryFlowIntegratedBC/vector_coefficient). The weights $\alpha$ and $\beta$
are set by [!param](/BCs/MFEMBoundaryFlowIntegratedBC/alpha) and
[!param](/BCs/MFEMBoundaryFlowIntegratedBC/beta), the latter defaulting to $\alpha/2$.

This term imposes the inflow value $f$ in DG discretizations of convection. It completes the
boundary flux added to the left hand side by
[MFEMDGTraceIntegratedBC](MFEMDGTraceIntegratedBC.md) or
[MFEMNonconservativeDGTraceIntegratedBC](MFEMNonconservativeDGTraceIntegratedBC.md), where it
plays the role of the missing neighbor value. Because it sits on the right hand side, its weights
must be the negatives of the $\alpha$ and $\beta$ of that left hand side term. The default
$\alpha = -1$, with $\beta = -1/2$, matches the default $\alpha = 1$, $\beta = 1/2$ of those
objects and contributes $-(\vec Q \cdot \hat n) f$ on the inflow boundary, where
$\vec Q \cdot \hat n < 0$, and nothing on outflow. The face term does not include $\rho$, so when
[!param](/BCs/MFEMDGTraceIntegratedBC/coefficient) is not one, supply $\rho f$ as $f$.

`createLFIntegrator()` returns an [`mfem::BoundaryFlowIntegrator`](https://docs.mfem.org/html/classmfem_1_1BoundaryFlowIntegrator.html),
which is added to the linear form with `AddBdrFaceIntegrator()`.

## Example Input File Syntax

!syntax parameters /BCs/MFEMBoundaryFlowIntegratedBC

!syntax inputs /BCs/MFEMBoundaryFlowIntegratedBC

!syntax children /BCs/MFEMBoundaryFlowIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
