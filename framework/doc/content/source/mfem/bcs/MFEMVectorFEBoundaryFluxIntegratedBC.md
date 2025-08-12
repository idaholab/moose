# MFEMVectorFEBoundaryFluxIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(f, \vec v \cdot \hat n)_{\partial\Omega} \,\,\, \forall v \in V

where $\vec v \in H(\mathrm{div})$, $f$ is a scalar coefficient, and $\hat n$ is the
outward facing unit normal vector on the boundary.

## Example Input File Syntax

!listing test/tests/mfem/kernels/darcy.i block=BCs

!syntax parameters /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

!syntax inputs /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

!syntax children /BCs/MFEMVectorFEBoundaryFluxIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
