# AddMFEMPreconditionerAction

!if! function=hasCapability('mfem')

## Summary

!syntax description /Preconditioner/AddMFEMPreconditionerAction

## Overview

Action called to add a linear preconditioner to an MFEM problem, parsing content inside a
[`Preconditioner`](source/mfem/solvers/MFEMSolverBase.md) block in the user input. Only has an effect if
the `Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=Problem FESpaces Preconditioner

!syntax parameters /Preconditioner/AddMFEMPreconditionerAction

!if-end!

!else
!include mfem/mfem_warning.md
