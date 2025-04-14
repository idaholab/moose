# MFEMVectorFunctionNormalDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFunctionNormalDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the normal
components of a $H(\mathrm{div})$ conforming vector FE at a boundary. The imposed value is
a function of space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/graddiv.i block=BCs

!syntax parameters /BCs/MFEMVectorFunctionNormalDirichletBC

!syntax inputs /BCs/MFEMVectorFunctionNormalDirichletBC

!syntax children /BCs/MFEMVectorFunctionNormalDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
