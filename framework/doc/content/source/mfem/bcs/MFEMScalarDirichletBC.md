# MFEMScalarDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMScalarDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) condition on a scalar variable on the
boundary, fixing its values to the input on the boundary.

## Example Input File Syntax

!listing test/tests/mfem/kernels/diffusion.i block=BCs

!syntax parameters /BCs/MFEMScalarDirichletBC

!syntax inputs /BCs/MFEMScalarDirichletBC

!syntax children /BCs/MFEMScalarDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
