# MFEMVectorNormalDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorNormalDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the normal
components of a $H(\mathrm{div})$ conforming vector FE at a boundary. The imposed value is
a coefficient that may vary in space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/graddiv.i block=BCs

!syntax parameters /BCs/MFEMVectorNormalDirichletBC

!syntax inputs /BCs/MFEMVectorNormalDirichletBC

!syntax children /BCs/MFEMVectorNormalDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
