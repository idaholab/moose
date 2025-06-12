# MFEMVectorTangentialDirichletBC

!if! function=hasCapability('mfem')

## Summary

!syntax description /BCs/MFEMVectorTangentialDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the tangential
components of a $H(\mathrm{curl})$ conforming vector FE at a boundary. The boundary value is
a coefficient that may vary in space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=BCs

!syntax parameters /BCs/MFEMVectorTangentialDirichletBC

!syntax inputs /BCs/MFEMVectorTangentialDirichletBC

!syntax children /BCs/MFEMVectorTangentialDirichletBC

!if-end!

!else
!include mfem/mfem_warning.md
