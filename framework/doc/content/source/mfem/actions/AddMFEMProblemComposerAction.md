# AddMFEMProblemComposerAction

!if! function=hasCapability('mfem')

## Overview

Action called to add a problem composer, an object that builds [ProblemOperator.md]s,
parsing content inside a [`ProblemComposers`](syntax/ProblemComposers/index.md) block in the user
input.
Only has an effect if the `Problem` type is set to [MFEMProblem.md].

## Example Input File Syntax

!listing test/tests/mfem/problemcomposers/explicit_composer_darcy.i block=Problem ProblemComposers

!listing test/tests/mfem/problemcomposers/explicit_composer_heat_transfer.i block=Problem ProblemComposers

!syntax parameters /ProblemComposers/AddMFEMProblemComposerAction

!if-end!

!else
!include mfem/mfem_warning.md
