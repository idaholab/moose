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
any `Executioner` stepping parameter. Under a [Transient.md] executioner each time step runs a
continuation of its own instead, which [#per-timestep] describes.

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

[!param](/Problem/ArcLengthProblem/lambda_max) is by default the only criterion that ends a path
successfully: the continuation runs until the load parameter reaches it.
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

[!param](/Problem/ArcLengthProblem/end_on_max_continuation_steps) makes a spent increment budget the
successful end of a path rather than a failed solve. Past the peak of a strain-softening material — a
damage model such as Mazars — the load parameter falls monotonically and never climbs back, so a
[!param](/Problem/ArcLengthProblem/lambda_max) beyond the peak is unreachable on that branch and no
stopping criterion is left for the path to meet. Tracing a softening response consequently ends as a
diverged solve however faithfully the trace followed the branch. Setting this parameter makes the
budget the designed end of the path instead: a continuation that runs its entire
[!param](/Problem/ArcLengthProblem/max_continuation_steps) budget is reported converged, and a path
whose corrector fails before the budget is spent still fails. To confirm a path ended at its budget
rather than at a failed corrector, re-run it with a larger
[!param](/Problem/ArcLengthProblem/max_continuation_steps): a genuine budget exit traces further,
while a corrector failure stops at the same point and fails the solve there. The two are otherwise
indistinguishable, a corrector failing in the last permitted increment being reported converged
exactly as a spent budget is.

The exit is read off the number of increments a continuation traced, so it needs a single solve per
path: a setup that solves the same path more than once carries the count past the budget and reports
the solve as failed. A [Transient.md] executioner takes the setting as well, where it ends the
continuation of every step rather than one whole path and changes what the load factor measures;
[#softening-transient] describes that regime.

This exit rests on PETSc's convergence reason alone, so the MOOSE-level checks that otherwise veto a
converged solve stop applying once the budget is spent: a [Terminator.md] with `fail_mode = SOFT`, an
invalid solution that [!param](/Problem/ArcLengthProblem/allow_invalid_solution) does not accept, and
an exception raised during the solve no longer fail it. A run relying on one of them to abort does
not abort here.

Sizing the budget is a physical choice as a result rather than a safety margin.
[!param](/Problem/ArcLengthProblem/lambda_max) has to be placed above the peak for the descending
branch to be reached at all — set below it, the path stops on the way up and never turns — so it goes
deliberately out of reach and [!param](/Problem/ArcLengthProblem/max_continuation_steps) becomes what
ends the path, setting how far down the branch the trace travels.

A Bratu source traced onto its descending branch pairs
[!param](/Problem/ArcLengthProblem/end_on_max_continuation_steps) with a
[!param](/Problem/ArcLengthProblem/lambda_max) put out of reach, composed by including
`bratu_source.i` and adjusting its continuation settings:

!listing test/tests/problems/arc_length/bratu_softening.i block=Problem

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
mesh and warp in the viewer: `use_displaced = true` fragments the output into one file per frame. A
transient run does not write these frames; see [#per-timestep].

An [AuxKernels] object takes the flag as well, which is what a material field animated this way
needs: left off it, the frames still render, carrying values from before the increment.

Objects scheduled on `LINEAR` run once per tangent-load assembly in addition to the ordinary residual
evaluations, because the load tag is reassembled every time PETSc asks for the tangent load.

!alert tip title=Recovering the history from a failed run
A diverged continuation writes no output at the end of the step. Adding
`additional_execute_on = 'failed'` to a CSV output flushes the history that was recorded up to the
point of failure, which is what shows whether the path was reflecting off
[!param](/Problem/ArcLengthProblem/lambda_min) or circling a closed loop.

## Per-timestep continuation id=per-timestep

Under a [Transient.md] executioner the continuation runs once per time step rather than once for the
whole run. Each step traces a path of its own with a step-local load parameter that goes from 0 to 1
over the load increment of that step alone, and the load factor the committed steps carry is added to
it:

\begin{equation}
R(u, \lambda) = F_\mathrm{int}(u) + \left(\Lambda + \lambda \, \Delta t\right) R_\mathrm{load}(u) = 0,
\end{equation}

where $\Lambda$ is the load factor accumulated by the steps already committed, $\lambda$ is the
step-local parameter the solver treats as the unknown, and $\Delta t$ is the time step size. The load
increment of a step is its time step size. As long as every step reaches the end of its increment and
the load factor only climbs, the load factor a step ends at is the time it ends at: [!param](/Executioner/Transient/dt) is the load
increment per step, and [!param](/Executioner/Transient/end_time) is the load factor the run finishes
at.

!listing test/tests/problems/arc_length/transient_cubic.i block=Executioner

That schedule applies the load in increments of 0.5 up to a load factor of 1 in this case.

State commits at the end of every step, so stateful materials advance along the path and irreversible
behaviour accumulates across steps, which a one-shot run cannot do because it commits nothing until
the whole path has been traced. The accumulated load factor is restartable data, so a run recovered
or restarted from a checkpoint resumes at the load factor of the last committed step and continues
the path from there.

!alert warning title=Explicit time integrators are not supported
Arc-length continuation is quasi-static path tracing: the continuation itself brings every step to
equilibrium. An explicit time integrator advances the problem state within a step, between its
stages, which commits the load increment of that step before the step has been traced. Nothing in the
code guards against this, because enumerating the explicit integrators is fragile and the combination
has no physical meaning. Use an implicit time integrator.

### Adapting and cutting back the load increment

A step whose continuation does not reach the end of its load increment within
[!param](/Problem/ArcLengthProblem/max_continuation_steps) increments reports the step unconverged.
The executioner rejects that step, the [TimeStepper](syntax/Executioner/TimeStepper/index.md) cuts
[!param](/Executioner/Transient/dt) back, and the step is retried with the smaller load increment
armed in its place. Cutting the time step cuts the load increment in the same proportion, so whatever
a `TimeStepper` does to the time step — growing it, shrinking it, cutting it back after a failure —
it does to the load increment.

A cutback does not, however, reduce the number of continuation increments a step needs when its load
target lies past a limit point. The arc length a step covers is measured on the solution and the
step-local load parameter together, and the solution excursion across a limit point is set by the
shape of the equilibrium path rather than by the size of the load increment: it does not shrink with
[!param](/Executioner/Transient/dt) at all. A step that has to cross a snap therefore costs about the
same number of increments however far the time step is cut back. Cutback rescues a mismatch between
the increment budget and the load target on the monotone stretches of a path, while
[!param](/Problem/ArcLengthProblem/max_continuation_steps) has to be sized to cross a snap within a
single step. Halving the time step is not a remedy for an under-sized increment budget at a limit
point — the retries shrink the increment down to [!param](/Executioner/Transient/dtmin) and the step
fails there instead.

### Step-local settings

[!param](/Problem/ArcLengthProblem/step_size),
[!param](/Problem/ArcLengthProblem/max_continuation_steps),
[!param](/Problem/ArcLengthProblem/psi_squared),
[!param](/Problem/ArcLengthProblem/correction_type) and
[!param](/Problem/ArcLengthProblem/lambda_min) each govern the continuation a single step traces and
are applied again from the start of every step. Two of them are measured against the step-local
parameter and so read differently than they do in a one-shot run:

- [!param](/Problem/ArcLengthProblem/step_size) is arc length in the step-local coordinates, whose
  load parameter spans 0 to 1 whatever the time step size is, so it is chosen against that fixed
  range rather than against [!param](/Executioner/Transient/dt).
- [!param](/Problem/ArcLengthProblem/lambda_min) clamps the step-local parameter, so it bounds how far
  a step may unload below the load factor it started from. The default of 0 keeps the load factor from
  falling below the value the step started at; tracing a path past its peak requires a negative value
  instead, which [#softening-transient] describes.

[!param](/Problem/ArcLengthProblem/lambda_max) is not a setting here. The step-local parameter spans
the increment of one step by construction, so the value it ends at is 1; any other value is an error
rather than a knob, and where the run finishes is set with
[!param](/Executioner/Transient/end_time) instead. No other `[Problem]` parameter changes meaning or
stops having an effect.

### Recording a transient path

Objects carrying `ARC_LENGTH_INCREMENT` still execute at every increment, so
[ArcLengthLoadParameter.md] and [ArcLengthHistory.md] sample the path at the same resolution they do
in a one-shot run. The `Arc length increment n, lambda = ...` banner still prints at each of those
boundaries, with the index restarting at zero in every step and the load factor reported as the
running total.

Writing a file per increment is disabled, though: the increment index stands in for the time on those
frames, and that pseudo-time interleaved with the times the steps advance through would corrupt the
sequence of an output. An [Outputs] sub-block carrying `ARC_LENGTH_INCREMENT` therefore writes nothing
under a transient executioner. Ordinary transient output is unaffected — each step writes a frame at
its own time, which is the load factor it reached — and tracing the path in one shot is the way to get
the per-increment animation.

### Choosing between the two modes

Trace a path per time step for:

- path-dependent behaviour, where plasticity, damage or any other stateful material has to advance
  along the path;
- a path that a fixed load increment cannot get through, where a cutback retries the step at a smaller
  increment;
- output on the real time axis, one frame per step.

Trace the whole path in one [Steady.md] solve for:

- a single complete equilibrium path of a path-independent problem, where committing nothing along the
  way costs nothing;
- the per-increment animation of the path.

### Tracing a softening path across time steps id=softening-transient

[!param](/Problem/ArcLengthProblem/end_on_max_continuation_steps) applies to a transient run as well,
where it ends the continuation of every step and is what carries a path past its peak. A step then has
two ways to finish: its step-local parameter reaches 1 and the step commits the whole load increment it
was given, or the step spends [!param](/Problem/ArcLengthProblem/max_continuation_steps) and commits
the net change it actually traced, which past the peak is a decrease. A step that does neither has
failed, and the [TimeStepper](syntax/Executioner/TimeStepper/index.md) cuts it back as it would any
other.

The increment a step applies carries a direction of travel with it, and a step that ends having moved
the load factor backward reverses that direction. The step after it therefore carries on the way the
path was going rather than back up the branch it just came down.

[!param](/Problem/ArcLengthProblem/lambda_min) has to be negative here and the problem errors if it is
not. It bounds how far a single step may unload below the load factor it started from, so the default
of zero forbids the descent this regime exists to trace.

The load factor stops equalling the time as a result. A step moves the load factor by up to
[!param](/Executioner/Transient/dt) of change in the direction it is travelling, so the two agree only
while the path is still climbing and every step reaches the end of its increment; a step that reverses,
or that ends short on a spent budget, parts them for the rest of the run. That is the intended reading
rather than a defect — time measures how far along the path the run has come rather than the level of
the load, and [!param](/Executioner/Transient/end_time) bounds how long the trace runs rather than
where it ends.

Choose between this and an ordinary transient run on whether the load the run is after is known to sit
below collapse. A load schedule needs it to be: the factor equals the time throughout and
[!param](/Executioner/Transient/end_time) is the load factor reached, which is predictable and simple
to set. Path tracing does not: the peak is what the run is looking for and the path descends past it,
which a schedule cannot represent at all because it advances monotonically by construction. Trace the
path when it goes past its peak and the run needs both real time steps and the committed, irreversible
state a damage or plasticity model accumulates along the way.

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
