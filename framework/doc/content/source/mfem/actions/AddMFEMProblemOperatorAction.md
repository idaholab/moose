# AddMFEMProblemOperatorAction

!if! function=hasCapability('mfem')

## Overview

Action called to add an MFEM Problem operator, it is only optional and automatically defaults
to the ProblemOperatorBuilderSteady for the MFEMSteady executioner and ProblemOperatorBuilderTransient
for the  MFEMTransient executioner. This block is mainly to be used in cases of custom problem 
operators. Only has an effect if the `Problem` type is set to [MFEMProblem.md].

## Example Input File Syntax

!listing test/tests/mfem/kernels/prob_op_block_heattransfer.i block=Problem ProblemOperatorBuilder
!listing test/tests/mfem/kernels/prob_op_block_darcy.i block=Problem ProblemOperatorBuilder

!syntax parameters /ProblemOperatorBuilder/AddMFEMProblemOperatorAction

!if-end!

!else
!include mfem/mfem_warning.md
