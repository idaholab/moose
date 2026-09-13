# MFEMWeakFormProblemComposerBase

!if! function=hasCapability('mfem')

## Overview

`MFEMWeakFormProblemComposerBase` is the base class of those [MFEMProblemComposer.md] classes whose
[ProblemOperator.md] solves an [EquationSystem.md] built by an object in the
[`WeakForms`](syntax/WeakForms/index.md) block. It adds the `weak_form` parameter naming that
object, and is the parent of `MFEMWeakFormProblemComposer`,
[MFEMComplexWeakFormProblemComposer.md], [MFEMTimeDependentWeakFormProblemComposer.md] and
[MFEMEigenWeakFormProblemComposer.md].

The `weak_form` parameter may be omitted only when the problem has a single weak form, whose
equation system is then used; if several are present and no name is given, the operator cannot
infer which system was intended and an error is raised.

Composers that build a problem operator with no associated weak form - such as a user-defined
custom operator - should derive from [MFEMProblemComposer.md] directly, and do not gain the
`weak_form` parameter.

!if-end!

!else
!include mfem/mfem_warning.md
