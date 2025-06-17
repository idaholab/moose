# MFEMVectorBoundaryIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorBoundaryIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f, \vec v)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $v \in \vec H^1$ and $\vec f$ is a vector coefficient of the same dimension.

## Example Input File Syntax

!syntax parameters /BCs/MFEMVectorBoundaryIntegratedBC

!syntax inputs /BCs/MFEMVectorBoundaryIntegratedBC

!syntax children /BCs/MFEMVectorBoundaryIntegratedBC

!if-end!

!else
!include mfem/mfem_warning.md
