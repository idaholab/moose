# MFEMVectorFunctionDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFunctionDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on all components of a
vector $H^1$ conforming variable on the boundary. The boundary value is a function of space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/linearelasticity.i block=BCs

!syntax parameters /BCs/MFEMVectorFunctionDirichletBC

!syntax inputs /BCs/MFEMVectorFunctionDirichletBC

!syntax children /BCs/MFEMVectorFunctionDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
