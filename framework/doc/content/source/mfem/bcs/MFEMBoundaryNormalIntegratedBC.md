# MFEMBoundaryNormalIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMBoundaryNormalIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f \cdot \hat n, v)_{\partial\Omega} \,\,\, \forall v \in V

where $v \in H^1$, $\vec f$ is a vector coefficient, and $\hat n$ is the outward facing unit normal
vector on the boundary.

!syntax parameters /BCs/MFEMBoundaryNormalIntegratedBC

!syntax inputs /BCs/MFEMBoundaryNormalIntegratedBC

!syntax children /BCs/MFEMBoundaryNormalIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
