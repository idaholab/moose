# MFEMVectorBoundaryFluxLFIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(f, \vec v \cdot \hat n)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $\vec v$ is a vector field whose components all lie in the same scalar $H^1$ space, $f$ is a scalar coefficient, and $\hat n$ is the outward facing unit normal vector on the boundary. The coefficient $f$ is set by [!param](/BCs/MFEMVectorBoundaryFluxLFIntegratedBC/coefficient).

`createLFIntegrator()` returns an [`mfem::VectorBoundaryFluxLFIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorBoundaryFluxLFIntegrator.html).

## Example Input File Syntax

!syntax parameters /BCs/MFEMVectorBoundaryFluxLFIntegratedBC

!syntax inputs /BCs/MFEMVectorBoundaryFluxLFIntegratedBC

!syntax children /BCs/MFEMVectorBoundaryFluxLFIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
