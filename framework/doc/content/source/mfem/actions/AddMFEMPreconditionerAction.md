# AddMFEMPreconditionerAction

## Summary

!syntax description /Preconditioner/AddMFEMPreconditionerAction

## Overview

Action called to add a linear preconditioner to an MFEM problem, parsing content inside a
`Preconditioner` block in the user input. Only has an effect if the `Problem` type is set to
`MFEMProblem`.

## Example Input File Syntax

!listing test/tests/kernels/curlcurl.i block=Problem FESpaces Preconditioner

!syntax parameters /Preconditioner/AddMFEMPreconditionerAction
