# MFEMFESpaceHierarchy

!if! function=hasCapability('mfem')

## Overview

`MFEMFESpaceHierarchy` builds an `mfem::ParFiniteElementSpaceHierarchy` from a
base MFEM finite element space. The hierarchy starts with the named coarsest
`FESpaces` object and then applies the ordered refinements listed in
[!param](/FESpaceHierarchies/MFEMFESpaceHierarchy/refinements).

Each refinement entry is either `h`, for one uniform h-refinement, or an integer
polynomial order for p-refinement. P-refinement orders must increase relative to
the current finest level.

The resulting hierarchy can be assigned to an `MFEMVariable` and used by
[MFEMGeometricMultigridSolver.md].

!syntax parameters /FESpaceHierarchies/MFEMFESpaceHierarchy

!syntax inputs /FESpaceHierarchies/MFEMFESpaceHierarchy

!syntax children /FESpaceHierarchies/MFEMFESpaceHierarchy

!if-end!

!else
!include mfem/mfem_warning.md
