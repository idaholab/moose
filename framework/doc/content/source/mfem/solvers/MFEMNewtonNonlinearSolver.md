# MFEMNewtonNonlinearSolver

!if! function=hasCapability('mfem')

## Overview

Defines and builds an `mfem::NewtonSolver` to solve nonlinear MFEM equation systems.

This solver requires Jacobian information from the MFEM operator and uses the externally configured
MFEM linear solver for the inner linear solves.

Define this object in the [`Solvers`](syntax/Solvers/index.md) block.

!syntax parameters /Solvers/MFEMNewtonNonlinearSolver

!syntax inputs /Solvers/MFEMNewtonNonlinearSolver

!syntax children /Solvers/MFEMNewtonNonlinearSolver

!if-end!

!else
!include mfem/mfem_warning.md
