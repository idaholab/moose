# MFEMNonlinearSolverBase

!if! function=hasCapability('mfem')

## Summary

Base class for MFEM nonlinear solve strategies.

## Overview

Classes derived from `MFEMNonlinearSolverBase` implement nonlinear solve strategies; the
`ConstructSolver` method should be overridden to construct the underlying `mfem::Solver` object.

`SetOperator` is called before a solve to supply the nonlinear operator, which corresponds to the MOOSE-MFEM `EquationSystem` object. For solvers that
report `RequiresExternalLinearSolver() == true`, `SetLinearSolver` is also called to supply the
configured linear solver for inner linear solves. Solvers that return `RequiresGradient() == true`
will additionally receive gradient information from the operator.

!if-end!

!else
!include mfem/mfem_warning.md
