# MFEMIntegratedBC

!if! function=hasCapability('mfem')

## Summary

Base class for objects applying integrated boundary conditions to an MFEM FE problem.

## Overview

Classes deriving from `MFEMIntegratedBC` represent boundary conditions which apply one or more
boundary integrators to the weak form on the set of user-specified boundaries.

`MFEMIntegratedBC` applies boundary integrator(s) to the weak form equation that is labeled
according to the test variable name returned from `getTestVariableName()`, similar to
[MFEMKernel.md] for domain integrators.

A discontinuous (L2) space has no degrees of freedom on boundary elements, so on such a test
variable the integrators are applied on the boundary faces of the elements next to the boundary
instead, as DG methods require. This lets boundary conditions such as
[MFEMBoundaryIntegratedBC.md] and [MFEMConvectiveHeatFluxBC.md] impose Neumann and Robin
conditions in DG problems.

!if-end!

!else
!include mfem/mfem_warning.md
