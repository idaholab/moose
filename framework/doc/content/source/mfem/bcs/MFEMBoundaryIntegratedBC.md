# MFEMBoundaryIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMBoundaryIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(f, v)_{\partial\Omega} \,\,\, \forall v \in V

where the test variable $v \in H^1$ and $f$ is a scalar coefficient. Often used for representing
Neumann-type boundary conditions.

!syntax parameters /BCs/MFEMBoundaryIntegratedBC

!syntax inputs /BCs/MFEMBoundaryIntegratedBC

!syntax children /BCs/MFEMBoundaryIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
