# MFEMVectorFEBoundaryFluxIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary integrator for integrating the bilinear form

!equation
(k \vec u \cdot \hat n, \vec v \cdot \hat n)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $\vec u$ and $\vec v$ are both in $H(\mathrm{div})$, $k$ is a scalar coefficient, and $\hat n$ is the outward facing unit normal vector on the boundary. The coefficient $k$ is set by [!param](/BCs/MFEMVectorFEBoundaryFluxIntegratedBC/coefficient).

`createBFIntegrator()` returns an [`mfem::VectorFEBoundaryFluxIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorFEBoundaryFluxIntegrator.html).

## Example Input File Syntax

!syntax parameters /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

!syntax inputs /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

!syntax children /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
