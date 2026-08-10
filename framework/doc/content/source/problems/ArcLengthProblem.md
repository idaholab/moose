# ArcLengthProblem

!syntax description /Problem/ArcLengthProblem

## Continuation along the equilibrium path

A prescribed load solves $F_\mathrm{int}(u) + R_\mathrm{load}(u) = 0$ for the solution $u$ alone, and
that has no answer past a limit point, beyond which the structure carries no more load. Arc-length
continuation makes the size of the load an unknown of the solve instead:

\begin{equation}
R(u, \lambda) = F_\mathrm{int}(u) + \lambda R_\mathrm{load}(u) = 0,
\end{equation}

where $F_\mathrm{int}$ is the residual assembled from the objects left in the default tags,
$R_\mathrm{load}$ is the residual assembled from the objects marked as the load, and $\lambda$ is the
load parameter. That extra scalar unknown needs one more equation, which is the arc-length
constraint: it fixes the distance travelled along the path per increment rather than the load carried
[!citep](riks1979, crisfield1981). Because the constraint measures the solution and the load
parameter together, the path is free to turn back on itself, so equilibrium points beyond a limit
point are reachable and the load may fall along the way.

`ArcLengthProblem` composes the residual above and delegates the constraint to PETSc's `SNESNEWTONAL`
solver, which it installs on the nonlinear solve. A whole path is traced within a single
[Steady.md] solve: continuation increments are internal to that solve and are not time steps, so the
number of them is bounded by [!param](/Problem/ArcLengthProblem/max_continuation_steps) rather than by
any `Executioner` stepping parameter.

## Marking the load

A load is an ordinary residual object — a [Kernels] object, a [BCs] object, a `DiracKernel` — whose
contribution is routed to the vector tag named by
[!param](/Problem/ArcLengthProblem/load_vector_tag) instead of to the default residual. Route it by
*replacing* the object's tags with `vector_tags`, which every residual object carries through
[TaggingInterface.md], as the examples in [#example] do.

`extra_vector_tags` appends the load tag while leaving the object in the default tags as well, which
would count its contribution once in $F_\mathrm{int}$ and again in $\lambda R_\mathrm{load}$; the
problem errors rather than let that happen. At least one object has to carry the load tag, otherwise
there is no load to continue in and the solve errors during setup.

Whether the load needs a second tag depends on how it responds to the solution:

- A *dead* load does not change with the solution — a fixed point force, or a pressure applied on the
  undeformed configuration. Its derivative with respect to the solution is zero, so `vector_tags`
  alone is enough.
- A *follower* or otherwise solution-dependent load does change with the solution — a pressure that
  follows a deforming surface, or a source term that is a function of the unknown. Its derivative
  belongs in the load Jacobian, so name [!param](/Problem/ArcLengthProblem/load_matrix_tag) in
  `matrix_tags` as well. The same double-count guard applies: replace the matrix tags rather than
  appending with `extra_matrix_tags`.

!alert warning title=Strongly enforced Dirichlet conditions at loaded degrees of freedom
A [DirichletBC.md] zeroes the residual row at each constrained degree of freedom, and it does so in
the default residual only. A load-tagged object that contributes at a constrained degree of freedom
leaves its contribution standing in the load tag, where $\lambda$ scales it back into the residual
after the row has been cleared. Either keep the load away from the constrained degrees of freedom —
an interior point load has this property — or enforce the constraint weakly with a
[PenaltyDirichletBC.md], which is the supported pattern when the load is nonzero there.

## Where the continuation stops

[!param](/Problem/ArcLengthProblem/lambda_max) is the only criterion that ends a path successfully:
the continuation runs until the load parameter reaches it.
[!param](/Problem/ArcLengthProblem/lambda_min) is a clamp rather than an exit — an increment that
would carry the load parameter below it is truncated back to it, and the path then sits at that value
until the increment budget runs out.

Both bounds consequently have to bracket the range of $\lambda$ the path actually travels through. A
[!param](/Problem/ArcLengthProblem/lambda_max) the path never reaches and a
[!param](/Problem/ArcLengthProblem/lambda_min) placed part-way into a descending branch fail the same
way: [!param](/Problem/ArcLengthProblem/max_continuation_steps) increments are spent and the solve
gives up. The two are distinguishable while they happen — a path pinned at the floor reflects off it
and ping-pongs, while a path whose ceiling sits above the top of a closed loop cycles around that loop
indefinitely — so a run that never terminates is worth plotting before the step size is blamed.

The step size is fixed: [!param](/Problem/ArcLengthProblem/step_size) is the arc length travelled per
increment for the whole solve and there is no adaptivity. It has to be small enough for the sharpest
turn on the path, and the budget large enough to cover the path at that size.

[!param](/Problem/ArcLengthProblem/correction_type) selects how a corrector iterate is returned to the
constraint. `exact` satisfies the constraint at every iteration by solving a quadratic, which may have
no real roots where the path turns sharply relative to
[!param](/Problem/ArcLengthProblem/step_size), and the increment then fails. `normal` corrects onto
the hyperplane normal to the current increment instead: cheaper per iteration, always a real
correction, and the iterate sits near the constraint surface rather than exactly on it. Switching to
`normal` is the alternative to shortening the step size at such a turn.

## Solver requirements

The continuation solves for the variation of the solution with respect to the load parameter using the
assembled Jacobian, so `solve_type = NEWTON` is required and a matrix-free type is an error.
`-snes_type` is owned by the problem and setting it through `petsc_options_iname` is an error as well:
PETSc applies the options database after the arc-length solver has been installed, so the option would
silently replace it and the solve would revert to load control. This also rules out the variational
inequality solver types, and therefore variable bounds, on an arc-length solve.
`residual_and_jacobian_together` is unsupported for a related reason — the fused assembly path
bypasses the tag split that the load parameter is applied through — and is guarded with an error.

Corrector iterations are judged by PETSc's own convergence test, which this problem installs in place
of the one MOOSE normally supplies, so a [Convergence system](syntax/Convergence/index.md) object has
no effect on an arc-length solve.

PETSc added `SNESNEWTONAL` in 3.22.0. On an older PETSc the problem errors at construction.

## Recording and animating the path

Every equilibrium state the continuation converges to is published on the `ARC_LENGTH_INCREMENT`
execution flag, and the console prints an `Arc length increment n, lambda = ...` banner at each of
those boundaries, so the stream of nonlinear iterations reads one increment at a time.
[ArcLengthLoadParameter.md] reports $\lambda$ at that state, and
[ArcLengthHistory.md] accumulates it alongside any postprocessors sampled at the same points, which
is the load-displacement curve such a run is usually asked for.

An [Outputs] sub-block carrying the same flag writes one frame per increment, turning the path into an
animation:

```
[Outputs]
  [path]
    type = Exodus
    execute_on = 'ARC_LENGTH_INCREMENT'
  []
[]
```

The whole continuation happens at a single solve time, so the index of the increment stands in for the
time on those frames and the pseudo-time in the file counts increments. Write them on the undisplaced
mesh and warp in the viewer: `use_displaced = true` fragments the output into one file per frame.

Objects scheduled on `LINEAR` run once per tangent-load assembly in addition to the ordinary residual
evaluations, because the load tag is reassembled every time PETSc asks for the tangent load.

!alert tip title=Recovering the history from a failed run
A diverged continuation writes no output at the end of the step. Adding
`additional_execute_on = 'failed'` to a CSV output flushes the history that was recorded up to the
point of failure, which is what shows whether the path was reflecting off
[!param](/Problem/ArcLengthProblem/lambda_min) or circling a closed loop.

## Example Input File Syntax id=example

A shallow circular arch loaded at its apex snaps through: the load rises to a limit point, falls while
the arch inverts, then rises again on the inverted branch. The load here is a point source, routed to
the load tag and carrying no matrix tag because a constant point force does not vary with the
deformation. It acts at an interior node, away from the clamped ends, so the strongly enforced
[DirichletBC.md] conditions on those ends do not overlap it.

!listing modules/solid_mechanics/test/tests/arc_length/arch_snapthrough.i block=DiracKernels

!listing modules/solid_mechanics/test/tests/arc_length/arch_snapthrough.i block=Problem

The bounds bracket the descending branch the arch passes through on its way to the inverted one, at
$\lambda_\mathrm{min} = -4$ and $\lambda_\mathrm{max} = 5$ in this case, and
[!param](/Problem/ArcLengthProblem/psi_squared) is zero, which selects the cylindrical constraint that
measures arc length with the displacements alone.

`modules/solid_mechanics/test/tests/arc_length/arch_snapback.i` is a shallower arch on pinned
supports, whose path snaps back — displacement as well as load decreases along part of it — which
neither load control nor displacement control can trace.

A volumetric source becomes a continuation load the same way. The Bratu source below is a function of
the unknown, so it is solution-dependent and carries the load matrix tag as well as the load vector
tag:

!listing test/tests/problems/arc_length/bratu_source.i block=Kernels

That source is nonzero at the constrained degrees of freedom, so the boundary conditions are enforced
with a penalty rather than strongly:

!listing test/tests/problems/arc_length/bratu_source.i block=BCs

!syntax parameters /Problem/ArcLengthProblem

!syntax inputs /Problem/ArcLengthProblem

!syntax children /Problem/ArcLengthProblem

!bibtex bibliography
