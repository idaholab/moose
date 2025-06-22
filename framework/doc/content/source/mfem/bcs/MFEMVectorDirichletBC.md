# MFEMVectorDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on all components of a
vector $H^1$ conforming variable on the boundary. The boundary value
is a coefficient that may vary in space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/linearelasticity.i block=BCs

!syntax parameters /BCs/MFEMVectorDirichletBC

!syntax inputs /BCs/MFEMVectorDirichletBC

!syntax children /BCs/MFEMVectorDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
