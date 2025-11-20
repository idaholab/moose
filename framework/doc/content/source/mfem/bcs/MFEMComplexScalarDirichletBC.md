# MFEMComplexScalarDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMComplexScalarDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) condition on
a complex-valued scalar variable on the boundary, fixing its values to the input
scalar coefficient on the boundary.

## Example Input File Syntax

!listing test/tests/mfem/complex/complex.i block=BCs

!syntax parameters /BCs/MFEMComplexScalarDirichletBC

!syntax inputs /BCs/MFEMComplexScalarDirichletBC

!syntax children /BCs/MFEMComplexScalarDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
