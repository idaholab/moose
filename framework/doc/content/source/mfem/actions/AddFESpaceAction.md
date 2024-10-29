# AddFESpaceAction

## Summary

!syntax description /FESpaces/AddFESpaceAction

## Overview

Action called to add an MFEM finite element space to the problem, parsing content inside an `FESpaces`
block in the user input. Only has an effect if the `Problem` type is set to `MFEMProblem`.

## Example Input File Syntax

!listing test/tests/kernels/heatconduction.i block=Problem FESpaces

!syntax description /FESpaces/AddFESpaceAction

!syntax parameters /FESpaces/AddFESpaceAction
