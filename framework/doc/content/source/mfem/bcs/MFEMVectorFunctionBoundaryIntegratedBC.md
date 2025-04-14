# MFEMVectorFunctionBoundaryIntegratedBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFunctionBoundaryIntegratedBC

## Overview

Adds the boundary integrator for integrating the linear form

!equation
(\vec f, \vec v)_{\partial\Omega} \,\,\, \forall \vec v \in V

where $v \in \vec H^1$ and $\vec f$ is a vector function of the same dimension.

## Example Input File Syntax

!syntax parameters /BCs/MFEMVectorFunctionBoundaryIntegratedBC

!syntax inputs /BCs/MFEMVectorFunctionBoundaryIntegratedBC

!syntax children /BCs/MFEMVectorFunctionBoundaryIntegratedBC

!else
!include mfem/mfem_warning.md
