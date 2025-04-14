# MFEMScalarFunctionDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMScalarFunctionDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) condition on
a scalar variable on the boundary, fixing its values to the input
scalar function on the boundary.

## Example Input File Syntax

!syntax parameters /BCs/MFEMScalarFunctionDirichletBC

!syntax inputs /BCs/MFEMScalarFunctionDirichletBC

!syntax children /BCs/MFEMScalarFunctionDirichletBC

!else
!include mfem/mfem_warning.md
