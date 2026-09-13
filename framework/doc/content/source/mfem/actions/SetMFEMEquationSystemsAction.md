# SetMFEMEquationSystemsAction

!if! function=hasCapability('mfem')

## Summary

Builds the MFEM `EquationSystem` objects solved by the problem from the
[`WeakForms`](syntax/WeakForms/index.md) added to it.

## Overview

Action executed during the `set_mfem_equation_systems` task, after all kernels, boundary
conditions and weak forms have been added. It calls each `MFEMWeakFormBase` derived object in turn
to construct and initialise its `EquationSystem`, and registers the result against the name of the
weak form that built it. Downstream objects that need a particular equation system - problem
composers and MFEM linear solvers - select it by that name through their `weak_form` parameter.

If no [`WeakForms`](syntax/WeakForms/index.md) block was added by the user, a single default weak
form is added first, chosen from the numeric type of the [MFEMProblem.md] and whether the
executioner in use is transient or steady state.

Only has an effect if the `Problem` type is set to [MFEMProblem.md].

!if-end!

!else
!include mfem/mfem_warning.md
