# Line Search System

Line searches choose how much of a nonlinear solver update is applied at each nonlinear iteration.
For Newton and Newton-Krylov solves, the nonlinear solver computes a search direction and the line
search chooses the step length along that direction. A line search can improve robustness for
strongly nonlinear problems, but they often add residual evaluations and thus may slow down some
models.

Use the [!param](/Executioner/Steady/line_search) parameter in the `Executioner` block to select a
line search:

!listing!
[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = bt
[]
!listing-end!

Most line search options available through MOOSE are PETSc SNES line searches. PETSc documents the
full API and command-line options in
[SNESLineSearchSetType](https://petsc.org/release/manualpages/SNES/SNESLineSearchSetType/) and in
the [SNES manual](https://petsc.org/release/manual/snes/#line-search-newton). The following
selections can be made in MOOSE:

| `line_search` | Source | Description |
| :- | :- | :- |
| `default` | PETSc | Leave the line search choice to PETSc for the selected SNES solver type. |
| `none` | PETSc | Alias for `basic` in MOOSE. Applies the update without an iterative globalization search. This can be useful when a line search prevents convergence. |
| `basic` | PETSc | Damped step line search. It applies the solver update, optionally scaled by PETSc damping options such as `-snes_linesearch_damping`. |
| `bt` | PETSc | Backtracking line search. This is PETSc's line search Newton default and is commonly used to reduce a step that increases the nonlinear residual too much. |
| `l2` | PETSc | Secant search using the L2 norm of the nonlinear function or objective function. In recent PETSc versions this is the `secant` line search implementation. |
| `cp` | PETSc | Critical-point line search. PETSc searches for a step length based on the directional condition \(F(x) \cdot Y\), where \(Y\) is the search direction. |
| `shell` | PETSc | PETSc shell line search type. In MOOSE this is mostly useful as infrastructure for MOOSE-provided custom line searches. |
| `contact` | MOOSE Contact module | See [contact line searches](ContactLineSearch.md optional=True). |

To inspect a line search during a run, pass PETSc options through the `Executioner` block:

!listing!
[Executioner]
  type = Steady
  line_search = bt
  petsc_options = '-snes_linesearch_monitor'
[]
!listing-end!

Custom MOOSE line searches are a developer extension point. They should inherit from `LineSearch`
and implement the line-searching operation.
