# MFEMComplexVectorNormalDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMComplexVectorNormalDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the normal
components of a complex-valued $H(\mathrm{div})$ conforming vector FE at a boundary. The imposed value is
a coefficient that may vary in space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/graddiv.i block=BCs

!syntax parameters /BCs/MFEMComplexVectorNormalDirichletBC

!syntax inputs /BCs/MFEMComplexVectorNormalDirichletBC

!syntax children /BCs/MFEMComplexVectorNormalDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
