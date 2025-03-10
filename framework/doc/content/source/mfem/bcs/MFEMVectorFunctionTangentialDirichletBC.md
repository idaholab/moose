# MFEMVectorFunctionTangentialDirichletBC

## Summary

!syntax description /BCs/MFEMVectorFunctionTangentialDirichletBC

## Overview

Boundary condition for enforcing an essential (Dirichlet) boundary condition on the tangential
components of a $H(\mathrm{curl})$ conforming vector FE at a boundary. The boundary value is
a function of space and/or time.

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=BCs

!syntax parameters /BCs/MFEMVectorFunctionTangentialDirichletBC

!syntax inputs /BCs/MFEMVectorFunctionTangentialDirichletBC

!syntax children /BCs/MFEMVectorFunctionTangentialDirichletBC
