# MFEMBoundaryCondition

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM boundary condition objects.

## Overview

`MFEMBoundaryCondition` represents boundary conditions applied to the MFEM problem, applied on the
user-provided list of boundaries.

Similar to `MFEMKernel`, the boundary condition is applied to the weak form equation that is labeled
according to the test variable name returned from `getTestVariableName()`.

`MFEMBoundaryCondition` objects come in two varieties, depending on which child class they inherit
from:

- Essential boundary conditions, inheriting from [`MFEMEssentialBC`](source/mfem/bcs/MFEMEssentialBC.md),
  for the application of Dirichlet-like BCs removing degrees of freedom from the problem at the
  boundary;

- Integrated boundary conditions, inheriting from
  [`MFEMIntegratedBC`](source/mfem/bcs/MFEMIntegratedBC.md), which apply one or more boundary integrators
  to the weak form on the specified boundaries.


!else
!include mfem/mfem_warning.md
