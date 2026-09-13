# AddMFEMWeakFormAction

!if! function=hasCapability('mfem')

## Summary

Adds an `MFEMWeakFormBase` derived object to an MFEM problem for each block in the
[`WeakForms`](syntax/WeakForms/index.md) syntax.

## Overview

Action called to add an `MFEMWeakFormBase` derived object to an MFEM problem, parsing content inside
a [`WeakForms`](syntax/WeakForms/index.md) syntax block in the user input. Only has an effect if the
`Problem` type is set to [MFEMProblem.md].

`MFEMWeakFormBase` derived objects are used to create MFEM `EquationSystem` objects from kernels and boundary
conditions that can be used as operators for downstream solvers and preconditioners. The equation
systems themselves are built later, during the `set_mfem_equation_systems` task, by
[SetMFEMEquationSystemsAction.md].

## Example Input File Syntax

!listing test/tests/mfem/weakforms/steady_weakform.i block=WeakForms

!if-end!

!else
!include mfem/mfem_warning.md
