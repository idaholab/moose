# AddMFEMSolverAction

!if! function=hasCapability('mfem')

## Overview

Action called to add an MFEM solver object to an MFEM problem, parsing content inside a
[`Solvers`](syntax/Solvers/index.md) block in the user input. Only has an effect if the
`Problem` type is set to [`MFEMProblem`](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/curlcurl.i block=Problem FESpaces Preconditioner Solvers

!syntax parameters /Solvers/AddMFEMSolverAction

!if-end!

!else
!include mfem/mfem_warning.md
