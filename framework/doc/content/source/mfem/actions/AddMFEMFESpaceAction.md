# AddMFEMFESpaceAction

## Summary

!syntax description /FESpaces/AddMFEMFESpaceAction

## Overview

Action called to add an MFEM finite element space to the problem, parsing content inside an
[`MFEMFESpace`](source/mfem/fespaces/MFEMFESpace.md) block in the user input. Only has an effect if the
`Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/heatconduction.i block=Problem FESpaces

!syntax parameters /FESpaces/AddMFEMFESpaceAction
