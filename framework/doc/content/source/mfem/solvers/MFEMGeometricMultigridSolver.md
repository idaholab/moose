# MFEMGeometricMultigridSolver

!if! function=hasCapability('mfem')

## Overview

`MFEMGeometricMultigridSolver` defines a geometric multigrid preconditioner
backed by `mfem::GeometricMultigrid`. It uses an
[MFEMFESpaceHierarchy.md] to define the multigrid levels, a coarse-level solver,
and one linear solver as the smoother of each remaining level.

A solver is set up for the operator of a single level, so each entry of
[!param](/Solvers/MFEMGeometricMultigridSolver/smoothers) must name a different
solver object, also distinct from
[!param](/Solvers/MFEMGeometricMultigridSolver/coarse_solver). A hierarchy of
three levels or more therefore needs as many smoother objects as levels above
the coarsest, even when they share a type and parameters.

The finest-level operator is supplied by the outer solver. Coarser level
operators are rediscretized from the equation system on each hierarchy level
using the requested [!param](/Solvers/MFEMGeometricMultigridSolver/assembly_levels).

This solver is intended for linear, single-variable equation systems. Nonlinear
equation systems and mixed bilinear form contributions are not supported.

## Example Input File Syntax

!listing test/tests/mfem/solvers/pmg_diffusion.i block=Solvers

!syntax parameters /Solvers/MFEMGeometricMultigridSolver

!syntax inputs /Solvers/MFEMGeometricMultigridSolver

!syntax children /Solvers/MFEMGeometricMultigridSolver

!if-end!

!else
!include mfem/mfem_warning.md
