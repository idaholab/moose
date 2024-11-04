# AddMFEMSolverAction

## Summary

!syntax description /Solver/AddMFEMSolverAction

## Overview

Action called to add a linear solver to an MFEM problem, parsing content inside a `Solver` block in
the user input. Only has an effect if the `Problem` type is set to `MFEMProblem`.

## Example Input File Syntax

!listing test/tests/kernels/curlcurl.i block=Problem FESpaces Preconditioner Solver

!syntax parameters /Solver/AddMFEMSolverAction
