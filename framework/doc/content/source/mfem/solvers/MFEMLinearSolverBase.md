# MFEMLinearSolverBase

!if! function=hasCapability('mfem')

## Summary

Base class for `mfem::Solver` objects used as linear solvers or preconditioners in MFEM problems.

## Overview

Classes derived from `MFEMLinearSolverBase` can be used as preconditioners or linear solvers; the
`ConstructSolver` method should be overridden to construct a `unique_ptr` to an `mfem::Solver`
derived object. The `GetSolver` method returns a reference to the underlying `mfem::Solver` for use
during a solve.

Problem-specific information - such as finite element spaces used in the set-up of some
preconditioners - can be passed to the `mfem::Solver` at construction time.

Linear solvers of type `T` that can be used as a Low-Order-Refined (LOR) preconditioner or solver
should inherit from the derived class `LORLinearSolverBase<T>`, as described in
[MFEMLORLinearSolverBase.md].

!if-end!

!else
!include mfem/mfem_warning.md
