# MFEMBoundaryTangentialLFIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec g \cdot \hat \tau, v)_{\partial\Omega} \,\,\, \forall v \in V

in 2D, where $v \in H^1$, $\vec g$ is a two-component vector coefficient, and $\hat \tau$ is the unit tangent vector on the boundary. The coefficient $\vec g$ is set by [!param](/BCs/MFEMBoundaryTangentialLFIntegratedBC/vector_coefficient).

`createLFIntegrator()` returns an [`mfem::BoundaryTangentialLFIntegrator`](https://docs.mfem.org/html/classmfem_1_1BoundaryTangentialLFIntegrator.html).

## Example Input File Syntax

!syntax parameters /BCs/MFEMBoundaryTangentialLFIntegratedBC

!syntax inputs /BCs/MFEMBoundaryTangentialLFIntegratedBC

!syntax children /BCs/MFEMBoundaryTangentialLFIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
