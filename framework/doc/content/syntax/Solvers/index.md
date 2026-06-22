# Solvers

The `Solvers` syntax defines named solver objects for a simulation.
Each sub-block under `Solvers` constructs one solver MooseObject and
adds it to the problem during action setup.

For MFEM-based problems, solver objects are used to configure both:

- linear solves, such as Krylov methods
- nonlinear solves, such as Newton and PETSc SNES-backed solvers

This syntax replaces the older singular `Solver` block. The plural
form is more flexible because a problem may need more than one solver
object at a time. Common examples include:

- a linear solver together with a preconditioner
- a nonlinear solver that drives the global solve and a separate linear
  solver used for Jacobian solves

## Structure

Each child block under `Solvers` names one solver object:

```text
[Solvers]
  [linear]
    type = MFEMHypreGMRES
    preconditioner = amg
    l_tol = 1e-12
  []
  [nonlinear]
    type = MFEMNewtonNonlinearSolver
    rel_tol = 1e-8
    abs_tol = 1e-10
  []
[]
```

The child block name is the object name. The `type` selects the solver
class and therefore the valid parameters for that block.

## MFEM Usage

For MFEM problems, solver objects are created through
`AddMFEMSolverAction` and stored directly on the `MFEMProblem`. Linear
and nonlinear solver objects share the same top-level syntax, but they
play different roles at solve time:

- linear solver objects wrap `mfem::Solver` implementations used for
  assembled linear systems
- nonlinear solver objects wrap nonlinear MFEM solve strategies, such
  as `mfem::NewtonSolver` or `mfem::PetscNonlinearSolver`

Some nonlinear solvers also depend on a separate linear solver. For
example, `mfem::NewtonSolver` requires the user to provide a Jacobian solver.
Others, such as the current `mfem::PetscNonlinearSolver` implementation, manage
their own internal linear solver stack.

## Why This Exists

The old singular `Solver` syntax encoded the assumption that an MFEM
input would have one distinguished solver block. That stopped scaling
once MFEM nonlinear solver objects were added, because the problem
could legitimately require multiple solver objects at once. Moving to
`Solvers` makes the syntax consistent with that reality and keeps the
solver configuration extensible.

## See Also

- [MFEM linear solver base class](source/mfem/solvers/MFEMLinearSolverBase.md)
- [MFEM nonlinear solver base class](source/mfem/solvers/MFEMNonlinearSolverBase.md)
