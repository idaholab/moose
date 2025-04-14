# MFEMVectorBoundaryIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorBoundaryIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f, \vec v)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $\vec v \in \vec H^1$ and $\vec f$ is a constant vector of the same dimension.

## Example Input File Syntax

!listing test/tests/mfem/kernels/linearelasticity.i block=BCs

!syntax parameters /BCs/MFEMVectorBoundaryIntegratedBC

!syntax inputs /BCs/MFEMVectorBoundaryIntegratedBC

!syntax children /BCs/MFEMVectorBoundaryIntegratedBC

!else
!include mfem/mfem_warning.md
