# MFEMScalarFunctorBoundaryIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMScalarFunctorBoundaryIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(f, v)_{\partial\Omega} \,\,\, \forall v \in V

where the test variable $v \in H^1$ and $f$ is a scalar coefficient. Often used for representing
Neumann-type boundary conditions.

!syntax parameters /BCs/MFEMScalarFunctorBoundaryIntegratedBC

!syntax inputs /BCs/MFEMScalarFunctorBoundaryIntegratedBC

!syntax children /BCs/MFEMScalarFunctorBoundaryIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
