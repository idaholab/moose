# AddMFEMProblemOperatorAction

## Summary

Set the ProblemOperator used in the MFEMExecutioner to solve the FE problem.

## Overview

Action called to add a [`ProblemOperator`](source/problem_operators/ProblemOperator.md) to the
problem from an MFEM executioner. This action is run as part of the MFEM problem setup if the
`Problem` type is set to [`MFEMProblem`](source/problem/MFEMProblem.md).
