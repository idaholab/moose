# MFEMVectorNormalDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorNormalDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the normal
components of a $H(\mathrm{div})$ conforming vector FE at a boundary. The imposed value is
constant in space and time.

## Example Input File Syntax

!syntax parameters /BCs/MFEMVectorNormalDirichletBC

!syntax inputs /BCs/MFEMVectorNormalDirichletBC

!syntax children /BCs/MFEMVectorNormalDirichletBC

!else
!include mfem/mfem_warning.md
