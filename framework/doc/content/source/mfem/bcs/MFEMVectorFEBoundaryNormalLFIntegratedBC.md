# MFEMVectorFEBoundaryNormalLFIntegratedBC

!if! function=hasCapability('mfem')

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f \cdot \hat n, \vec v \cdot \hat n)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $\vec v \in H(\mathrm{div})$, $\vec f$ is a vector coefficient, and $\hat n$ is the outward facing unit normal vector on the boundary. The coefficient $\vec f$ is set by [!param](/BCs/MFEMVectorFEBoundaryNormalLFIntegratedBC/vector_coefficient).

`createLFIntegrator()` returns an [`mfem::VectorFEBoundaryNormalLFIntegrator`](https://docs.mfem.org/html/classmfem_1_1VectorFEBoundaryNormalLFIntegrator.html).

## Example Input File Syntax

!syntax parameters /BCs/MFEMVectorFEBoundaryNormalLFIntegratedBC

!syntax inputs /BCs/MFEMVectorFEBoundaryNormalLFIntegratedBC

!syntax children /BCs/MFEMVectorFEBoundaryNormalLFIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
