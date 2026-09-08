# SetMFEMProblemOperatorsAction

!if! function=hasCapability('mfem')

## Summary

Builds the MFEM `ProblemOperator` objects executed by the problem from the
[`ProblemComposers`](syntax/ProblemComposers/index.md) added to it.

## Overview

Action executed during the `set_mfem_problem_operators` task, after the equation systems have been
built by [SetMFEMEquationSystemsAction.md] and the MFEM solvers have been resolved. It calls each
`MFEMProblemComposer` derived object in turn to create its problem operator, then initialises every
operator against the problem's true-DoF solution vector.

If no problem composer was added by the user, a single default composer is added first, chosen from
the numeric type of the [MFEMProblem.md] and whether the executioner in use is transient or steady
state.

Only has an effect if the `Problem` type is set to [MFEMProblem.md].

!if-end!

!else
!include mfem/mfem_warning.md
