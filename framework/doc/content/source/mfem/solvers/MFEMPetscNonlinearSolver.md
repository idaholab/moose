# MFEMPetscNonlinearSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::PetscNonlinearSolver` to solve nonlinear MFEM equation systems
through PETSc SNES.

This solver currently requires Jacobian information from the MFEM operator and manages its own
internal PETSc KSP/PC stack rather than using an external MFEM linear solver.

PETSc options may be supplied through the object parameters, and `petsc_options_prefix` controls
the prefix applied to the owned SNES object and its sub-objects.

Define this object in the [Solvers](source/mfem/solvers/MFEMSolverBase.md) block.

!syntax parameters /Solvers/MFEMPetscNonlinearSolver

!syntax inputs /Solvers/MFEMPetscNonlinearSolver

!syntax children /Solvers/MFEMPetscNonlinearSolver

!if-end!

!else
!include mfem/mfem_warning.md
