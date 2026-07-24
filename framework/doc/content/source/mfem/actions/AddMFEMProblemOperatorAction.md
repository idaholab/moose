# AddMFEMProblemOperatorAction

!if! function=hasCapability('mfem')

## Overview

Action called to add a MFEMProblemComposer that builds ProblemOperators in the executioner.
[`MFEMProblemComposer`](syntax/MFEMProblemComposer/index.md) block in the user input.
Only has an effect if the `Problem` type is set to
[MFEMProblem.md](source/mfem/problem/MFEMProblem.md).

## Example Input File Syntax

!listing test/tests/mfem/kernels/prob_op_block_darcy.i block=Problem MFEMProblemComposer
!listing test/tests/mfem/kernels/prob_op_block_heattransfer.i block=Problem MFEMProblemComposer

!syntax parameters /MFEMProblemComposer/AddMFEMProblemOperatorAction

!if-end!

!else
!include mfem/mfem_warning.md
