# MFEMVectorFunctorTangentialDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorFunctorTangentialDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the tangential
components of a $H(\mathrm{curl})$ conforming vector FE at a boundary. The boundary value is
a function of space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=BCs

!syntax parameters /BCs/MFEMVectorFunctorTangentialDirichletBC

!syntax inputs /BCs/MFEMVectorFunctorTangentialDirichletBC

!syntax children /BCs/MFEMVectorFunctorTangentialDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
