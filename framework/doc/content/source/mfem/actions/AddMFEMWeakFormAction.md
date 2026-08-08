# AddMFEMWeakFormAction

!if! function=hasCapability('mfem')

## Overview

Action called to add an `MFEMWeakFormBase` derived object to an MFEM problem, parsing content inside
a [`WeakForm`](syntax/WeakForms/index.md) syntax block in the user input. Only has an effect if the
`Problem` type is set to [MFEMProblem.md].

`MFEMWeakFormBase` derived objects are used to create MFEM `EquationSystem` objects from kernels and boundary
conditions that can be used as operators for downstream solvers and preconditioners.

## Example Input File Syntax

!listing test/tests/mfem/weakforms/steady_weakform.i block=WeakForms

!if-end!

!else
!include mfem/mfem_warning.md
