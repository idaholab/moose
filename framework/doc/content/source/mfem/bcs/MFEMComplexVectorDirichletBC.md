# MFEMComplexVectorDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMComplexVectorDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on all components of a
complex-valued vector $H^1$ conforming variable on the boundary. The boundary value
is a coefficient that may vary in space and/or time.

!syntax parameters /BCs/MFEMComplexVectorDirichletBC

!syntax inputs /BCs/MFEMComplexVectorDirichletBC

!syntax children /BCs/MFEMComplexVectorDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
