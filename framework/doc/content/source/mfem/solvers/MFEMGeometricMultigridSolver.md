# MFEMGeometricMultigridSolver

!if! function=hasCapability('mfem')

## Overview

`MFEMGeometricMultigridSolver` defines a geometric multigrid preconditioner
backed by `mfem::GeometricMultigrid`. It uses an
[MFEMFESpaceHierarchy.md] to define the multigrid levels, a coarse-level solver,
and one or more linear solvers as smoothers for the remaining levels.

The finest-level operator is supplied by the outer solver. Coarser level
operators are rediscretized from the equation system on each hierarchy level
using the requested [!param](/Solvers/MFEMGeometricMultigridSolver/assembly_levels).

This solver is intended for single-variable equation systems. Mixed bilinear form
contributions are not supported.

!syntax parameters /Solvers/MFEMGeometricMultigridSolver

!syntax inputs /Solvers/MFEMGeometricMultigridSolver

!syntax children /Solvers/MFEMGeometricMultigridSolver

!if-end!

!else
!include mfem/mfem_warning.md
