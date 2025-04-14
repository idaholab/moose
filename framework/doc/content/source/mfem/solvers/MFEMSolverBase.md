# MFEMSolverBase

!if! function=hasCapability('mfem')

## Summary

Base class for `mfem::Solver` objects to use in MFEM problems.

## Overview

Classes derived from `MFEMSolverBase` can usually be used as preconditioners or linear solvers; the
`constructSolver` method should be overridden to construct a `shared_ptr` to an `mfem::Solver`
derived object, and the `getSolver` method should return the `shared_ptr` for use during a solve.

Problem-specific information - such as finite element spaces used in the set-up of some
preconditioners - can be passed to the `mfem::Solver` at construction time.


!else
!include mfem/mfem_warning.md
