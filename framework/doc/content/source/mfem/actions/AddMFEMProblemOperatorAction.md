# AddMFEMProblemOperatorAction

!if! function=hasCapability('mfem')

## Overview

Action called to add a ProblemOperatorBuilder that builds ProblemOperators in the executioner.
[`ProblemOperatorBuilder`](source/syntax/ProblemOperatorBuilder/index.md) block in the user input.
Only has an effect if the `Problem` type is set to
[MFEMProblem.md](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/prob_op_block_darcy.i block=Problem ProblemOperatorBuilder
!listing test/tests/mfem/kernels/prob_op_block_heattransfer.i block=Problem ProblemOperatorBuilder

!syntax parameters /ProblemOperatorBuilder/AddMFEMProblemOperatorAction

!if-end!

!else
!include mfem/mfem_warning.md
