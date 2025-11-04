# AddMFEMSolverAction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Solver/AddMFEMSolverAction

## Overview

Action called to add a linear solver to an MFEM problem, parsing content inside a
[`Solver`](source/mfem/solvers/MFEMSolverBase.md) block in the user input. Only has an effect if the
`Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=Problem FESpaces Preconditioner Solver

!syntax parameters /Solver/AddMFEMSolverAction

!if-end!

!else
!include mfem/mfem_warning.md
