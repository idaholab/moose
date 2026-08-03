# AddMFEMProblemComposerAction

!if! function=hasCapability('mfem')

## Overview

Action called to add a problem composer, an object that builds [ProblemOperator.md]s,
parsing content inside a [`ProblemComposer`](syntax/ProblemComposer/index.md) block in the user
input.
Only has an effect if the `Problem` type is set to [MFEMProblem.md].

## Example Input File Syntax

!listing test/tests/mfem/problemcomposers/prob_op_block_darcy.i block=Problem ProblemComposer

!listing test/tests/mfem/problemcomposers/prob_op_mfem_multiple_timesequences.i block=Problem ProblemComposer

!listing test/tests/mfem/problemcomposers/prob_op_block_heattransfer.i block=Problem ProblemComposer

!syntax parameters /MFEMProblemComposer/AddMFEMProblemComposerAction

!if-end!

!else
!include mfem/mfem_warning.md
