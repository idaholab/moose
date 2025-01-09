# AddFESpaceAction

## Summary

!syntax description /FESpaces/AddFESpaceAction

## Overview

Action called to add an MFEM finite element space to the problem, parsing content inside an
[`MFEMFESpace`](source/fespaces/MFEMFESpace.md) block in the user input. Only has an effect if the
`Problem` type is set to [`MFEMProblem`](source/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/kernels/heatconduction.i block=Problem FESpaces

!syntax parameters /FESpaces/AddFESpaceAction
