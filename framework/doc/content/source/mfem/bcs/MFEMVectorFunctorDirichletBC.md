# MFEMVectorFunctorDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFunctorDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on all components of a
vector $H^1$ conforming variable on the boundary. The boundary value is a functor of space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/linearelasticity.i block=BCs

!syntax parameters /BCs/MFEMVectorFunctorDirichletBC

!syntax inputs /BCs/MFEMVectorFunctorDirichletBC

!syntax children /BCs/MFEMVectorFunctorDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
