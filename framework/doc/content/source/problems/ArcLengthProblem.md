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

!alert warning title=Constant loads have to start in equilibrium
A load left in the default tags is carried at full strength from the first increment, while the
continuation looks for an equilibrium point within one arc length of the state the solve starts from.
A constant load applied to a state that is not already in equilibrium with it puts no such point in
reach, and the corrector settles on the nearest state it can find instead — characteristically a
negative load factor that cancels the constant load back out. It converges, and it means nothing.
Equilibrate the constant loads with an ordinary solve first and start the continuation from that
state.

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
[!param](/Problem/ArcLengthProblem/max_continuation_steps) budget through converged increments is
reported converged, and a path whose corrector fails inside any increment — the last permitted one
included — still fails. The two endings share the solver's exit reason and are told apart by the
nonlinear iteration count at the exit: a spent budget is only reached through a converged
increment, which leaves the count short of the iteration cap, while a corrector stopped by the cap
sits exactly at it.

The exit is read off the number of increments a continuation traced, so it needs a single solve per
path: a setup that solves the same path more than once carries the count past the budget and reports
the solve as failed. The setting belongs to a one-shot path alone — a transient run advances by a
single increment per step, whose internal budget is always a designed ending, and errors when the
input sets this; [#per-timestep] describes that mode.

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

## Stepping along the path id=per-timestep

Under a [Transient.md] executioner every time step advances the trace by a single continuation
increment and commits the state that increment reaches, which is the classical incremental
arc-length method of Riks and Crisfield with PETSc's solver as the engine of each increment. The
step solves

\begin{equation}
R(u, \lambda) = F_\mathrm{int}(u) + \left(\Lambda + \lambda \, \Delta\right) R_\mathrm{load}(u) = 0,
\end{equation}

where $\Lambda$ is the load factor the committed steps carry, $\lambda$ is the step-local parameter
the solver treats as the unknown, and $\Delta$ is the time step size signed by the direction of
travel. The committed load factor moves by the part of $\Delta$ the increment traverses, which may
be any fraction of it and either sign, so the trace climbs where the path climbs and sheds load
where it descends. Time is a pseudo parameter that counts arc steps: the load factor of the trace
is what [ArcLengthLoadParameter.md] reports, and it parts from the time at the first turn of the
path.

!listing test/tests/problems/arc_length/transient_path.i block=Problem Executioner

State commits at the end of every step, which here is the end of every increment, so stateful
materials advance along the path itself and irreversible behaviour accumulates increment by
increment, including down a descending branch. That ordering is what a history dependent material
needs. A solve that crosses a falling stretch of path in one span — a one-shot continuation, or any
scheme whose increments span whole excursions — evaluates the history only at the state it ends at,
so the excursion never registers in the material and the committed trace is stiffer than the path
it claims to have crossed. The accumulated load factor and the direction of travel are restartable
data, so a run recovered or restarted from a checkpoint resumes the trace where it left it.

A step whose increment converges short of its whole load span — the radius bounding it at a turn —
commits the fraction it traced, and that ending is told apart from a corrector that burned out by
the nonlinear iteration count at the exit: the solver consults its budget only at the boundary a
converged increment opens, so a genuine budget stop leaves the count short of the iteration cap,
while a corrector stopped by the cap itself sits exactly at it and the attempt is failed rather
than committed off equilibrium.

!alert warning title=Explicit time integrators are not supported
Arc-length continuation is quasi-static path tracing: the continuation itself brings every step to
equilibrium. An explicit time integrator advances the problem state within a step, between its
stages, which commits the load increment of that step before the step has been traced. Nothing in the
code guards against this, because enumerating the explicit integrators is fragile and the combination
has no physical meaning. Use an implicit time integrator.

### Cutbacks shrink the load span, not the radius

The arc length of the increment a step takes is [!param](/Problem/ArcLengthProblem/step_size) at
every time step size. A step whose increment fails — a corrector that overshoots a sharp turn
characteristically converges cleanly step after step on the approach and diverges at the turn
itself — is rejected, the [TimeStepper](syntax/Executioner/TimeStepper/index.md) cuts the time
step back, and the retry covers a smaller load span with the same turning radius. The two are
kept apart deliberately: the solution excursion across a sharp turn is set by the shape of the
path and does not shrink with the load span, so a retry that shrank the radius with the span
would confine the corrector exactly when the retry needs its reach.

The radius sets the resolution of the trace as well as its robustness. An increment whose arc
sphere spans a whole feature of the path — a serration of a crack advance, a small snap — converges
past it in one committed jump: the state it lands on is an equilibrium and the history committed is
the history of that jump, so a coarse radius surveys the path quickly at the cost of resolving its
finest excursions, and a fine radius resolves them at the cost of more steps. The margin is real: a
radius a factor of two above the one that threads a serration field can fail its first tooth at
every load span, so size the radius on the finest feature of the path rather than on the smooth
stretches.

### Non-smooth material events and lagged updates

A material whose evolution has a corner — the onset of a bilinear cohesive law, a damage cap, an
element exhausted — puts a facet in the residual, and a Newton corrector can limit-cycle across a
facet without ever converging, at every load span a cutback ladder offers: the failure lives in the
iteration itself, so nothing the stepping does removes it. Pair the continuation with the lagged
update the material offers — `use_old_damage` on a damage model, `lag_displacement_jump` on a
cohesive law — so every increment linearizes a smooth problem with the evolution frozen at the last
committed state. The per-step commits are what bound the lag: the history runs at most one
committed increment behind the path, a distance the radius sizes, where the same lag under a
prescribed ramp trails by a whole ramp increment.

### Settings a transient run owns

[!param](/Problem/ArcLengthProblem/step_size),
[!param](/Problem/ArcLengthProblem/psi_squared) and
[!param](/Problem/ArcLengthProblem/correction_type) govern every increment as they govern a
one-shot continuation. [!param](/Problem/ArcLengthProblem/max_continuation_steps),
[!param](/Problem/ArcLengthProblem/end_on_max_continuation_steps),
[!param](/Problem/ArcLengthProblem/lambda_max) and
[!param](/Problem/ArcLengthProblem/lambda_min) belong to a one-shot path alone, and a transient run
errors when the input sets one: every step advances by a single increment, so the run owns the step
budget and the load parameter clamps. The step-local parameter is capped at the step's own
increment of loading and floored at a fixed span of physical unloading sized from the nominal
increment, so cutbacks shrink what a step may load without confining how deep the corrector may
reach across a turn, and a descending stretch is traced across steps. Where the run finishes is set
with
[!param](/Executioner/Transient/end_time) or `num_steps`, which bound how long the trace runs, or
with a [Terminator.md] watching [ArcLengthLoadParameter.md], which ends it at a load factor or any
other measure of the state.

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
its own time — and tracing the path in one shot is the way to get
the per-increment animation.

### Switching between plain solves and the continuation

A path often begins with a stretch that plain Newton handles at a solve per step — an elastic climb,
a preload ramp — and only later needs continuation.
[!param](/Problem/ArcLengthProblem/use_continuation) puts the switch inside one run: a step solved
with it false is an ordinary Newton solve with the whole of its load increment applied as a
prescribed ramp along the current direction of travel, so the load factor equals the time and no
continuation increments are spent, and the first step solved with it true opens a continuation at
the committed load factor. The trace is continuous through every switch, and the two-input pattern
this replaces — ramp with plain solves, write a checkpoint, restart the continuation from it — is
no longer needed when the preparation is the same tagged load ramped. A preparation with different
physics, such as a separate constant load that has to be equilibrated first, still belongs in a run
of its own ahead of a restart.

The parameter is controllable, and a [Controls](syntax/Controls/index.md) object that writes it
decides step by step which regime solves: a [BoolFunctionControl.md] switches on a function of
time, and a custom control can read a postprocessor and hand the run to the continuation when,
say, a nonlinear iteration count or a damage measure crosses a threshold.

!listing modules/solid_mechanics/test/tests/arc_length/sent_arclength.i block=Controls

### Choosing between the two modes

Step along the path under [Transient.md] for:

- path-dependent behaviour, where plasticity, damage or any other stateful material has to advance
  along the path — the history is committed at every increment, which a solve that spans whole
  excursions cannot do;
- a path with sharp turns, where a failed step hands the retry a smaller load span at the same
  radius;
- output one frame per committed state.

Trace the whole path in one [Steady.md] solve for:

- a single complete equilibrium path of a path-independent problem, where committing nothing along the
  way costs nothing and the predictor carries its direction internally;
- the per-increment animation of the path.

### The direction of travel and the dissipation guard id=softening-transient

The direction the path is travelled in has to survive a step boundary, and PETSc's predictor cannot
carry it there: its memory ends with the solve, and a fresh solve opens its first increment with a
sign choice made from the state it starts at. Near a limit point that choice can walk a step
backward along the branch just traced. Such a step converges — the branch it retraces is made of
equilibrium points — so the outcome of the solve cannot expose it, and the energy the step
dissipates is what does [!citep](verhoosel2009). A descent along the path of a dissipative
structure sheds load because the structure dissipates; unloading it elastically, with the
irreversible state held by its own irreversibility, descends without dissipating, and the elastic
unload of a linear structure is proportional, which cancels the dissipation increment
$\tfrac{1}{2}\left(\Lambda\, R_\mathrm{load}^T \Delta u - \Delta\lambda\, R_\mathrm{load}^T u\right)$
exactly. A converged step whose net load change runs downward without dissipating is therefore
failed, its retry travels the other way, and the console says so:

```
Arc length step descended without dissipating, which is a walk back down an
elastic branch rather than a descent along the path, so the attempt is failed
and the retry travels the other way.
```

The problem also remembers a direction of travel of plus or minus one across steps: a step whose
committed change ran against it turns it around, so a trace that has turned a genuine fold keeps
descending rather than reading the next step as a climb back up the branch it just came down.

The guard is decisive for dissipative physics — damage, cohesive fracture, plasticity — which is
what stepping along the path with committed history exists for. On a purely elastic nonlinear path
a retrace dissipates nothing *and* descends non-proportionally, so the measure cannot separate the
two there; the direction memory still bounds a wrong turn, and the one-shot continuation, whose
predictor carries its direction internally, is the mode of choice for such paths.

Choose between stepping along the path and an ordinary transient run on whether the load the run is
after is known to sit below collapse. A load schedule needs it to be: the prescribed factor ramps
with the time, which is predictable and simple to set, and has nothing to converge to past a limit
point. Stepping along the path does not: the peak is what the run is looking for, and the trace
commits the descent past it, with the irreversible state a damage or plasticity model accumulates
committed along the way.

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
