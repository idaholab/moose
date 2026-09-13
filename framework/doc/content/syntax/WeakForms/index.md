# WeakForms

!if! function=hasCapability('mfem')

The `WeakForms` syntax is used to create MFEM `MFEMWeakFormBase`-derived objects to set up MFEM
[EquationSystem.md] operators that can be used by downstream solvers and preconditioners. If no
`WeakForms` block has been added to an input file by the user, a default `MFEMWeakFormBase`-derived
object will be added to the system, based on the `NumericType` (real or complex) used by the
`MFEMProblem`, and whether the Executioner in use is transient or steady state.

Each weak form selects the kernels and boundary conditions that contribute to its equation system
through its `kernels` and `bcs` parameters, which name blocks in the `Kernels` and `BCs` blocks
respectively. If either parameter is omitted, *all* kernels or boundary conditions added in the
input file are used, which is the behaviour of the default weak form.

!listing test/tests/mfem/weakforms/steady_weakform.i block=WeakForms

## Selecting between multiple weak forms

More than one weak form may be added, in which case the equation system built by each is registered
under the name of the block that created it. Objects that consume an equation system - the
[ProblemComposers](syntax/ProblemComposers/index.md) that build problem operators, and MFEM linear
solvers and preconditioners - then select one by name using their `weak_form` parameter. That
parameter may be omitted only when the problem has a single weak form; if it is omitted while
several are present, the object cannot infer which system was intended and an error is raised.

!alert note
On a solver, `weak_form` selects only the equation system that solver configures *itself* against,
such as the system [MFEMGeometricMultigridSolver.md] builds its hierarchy from. It does not bind
the solver to a particular problem operator: a problem still has a single driver solver, which is
used by every problem operator regardless of which equation system that operator solves. In a
problem with several weak forms, a solver whose configuration depends on its equation system should
therefore be used only when all of the operators solve the system it names.

!if-end!

!else
!include mfem/mfem_warning.md
