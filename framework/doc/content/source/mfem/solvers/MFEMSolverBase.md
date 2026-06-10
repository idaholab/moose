# MFEMSolverBase

!if! function=hasCapability('mfem')

## Summary

Common base class for all MFEM solver objects.

## Overview

`MFEMSolverBase` is the common base class for all MFEM solver objects. Derived classes must
implement `ConstructSolver` to create the underlying `mfem::Solver` object. The `GetSolver`
method returns a reference to that solver for use during a solve.

Direct subclasses are [MFEMLinearSolverBase.md], which covers linear solvers and preconditioners,
and [MFEMNonlinearSolverBase.md], which covers nonlinear solve strategies.

!if-end!

!else
!include mfem/mfem_warning.md
