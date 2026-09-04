# Remeshing System

The `[Remeshing]` block replaces elements of the mesh during a transient simulation, and optionally
moves the mesh between those replacements. The executioner drives it once per time step, after the
time step size is known and before the solve, so every step is solved on the mesh the block just
produced.

The block holds its objects in three nested sub-blocks:

- [Criteria](syntax/Remeshing/Criteria/index.md) decide when a replacement happens. At least one
  criterion is required, and a replacement happens when any of them fires.
- [Remeshers](syntax/Remeshing/Remeshers/index.md) perform the replacement. At least one remesher is
  required; several are applied in turn, each seeing the mesh the one before it left.
- [Smoothers](syntax/Remeshing/Smoothers/index.md) move the mesh between replacements. Exactly one
  smoother is required when [!param](/Remeshing/RemeshingAction/mesh_movement) is `true`, and none
  is accepted when it is not.

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing

## Operating Modes

[!param](/Remeshing/RemeshingAction/mesh_movement) and
[!param](/Remeshing/RemeshingAction/displacements) select how the mesh moves and which
configuration the criteria measure.

Setting [!param](/Remeshing/RemeshingAction/mesh_movement) to `true` selects arbitrary
Lagrangian-Eulerian (ALE) mesh movement: the smoother moves the nodes of the mesh itself every step,
the problem is discretized on the moved coordinates, and there is no displacement field. This is the
mode of the input above, where a [LaplaceSmoother.md] carries a prescribed interface velocity into
the interior.

Naming the displacement variables of the problem in
[!param](/Remeshing/RemeshingAction/displacements) selects true displacements: the criteria are
evaluated on the displaced mesh, while the surgery is performed on the reference mesh, which keeps
the description total Lagrangian. The displacement variables are then carried onto the new elements
like any other variable.

!listing test/tests/remeshing/true_disp_quality_remesh.i block=Remeshing

Setting neither leaves the mesh where the mesh generators built it and remeshes on whatever the
criteria measure there, which improves a mesh whose element quality starts out below the threshold.

The two settings are independent. A problem may have displacement variables in its `[Mesh]` block,
so that it carries a displaced mesh, and still move its reference mesh with a smoother.

## Mesh Motion and the Reference Configuration id=motion

The mesh position is bookkeeping rather than a degree of freedom of the problem, and the motion a
smoother writes is tracked in total since the last replacement rather than accumulated in place. Let
$\mathbf{X}_0$ be the node coordinates snapshotted at the last replacement and $\mathbf{d}$ the
pseudo-displacement the smoother has written since then. Every step the engine sets the coordinates
of every node to

\begin{equation}
\mathbf{x} = \mathbf{X}_0 + \mathbf{d} .
\end{equation}

A smoother writes the total $\mathbf{d}$ since the snapshot, never an increment, so the coordinates
are recomputed from scratch instead of being added to. At a replacement $\mathbf{X}_0$ becomes the
current $\mathbf{x}$ and $\mathbf{d}$ returns to zero, which is exact because nothing was ever
accumulated in place. [MaxPseudoDisplacement.md] reports $\max_n \lVert \mathbf{d}_n \rVert$ and so
drops back to zero on the step a replacement occurs; [RemeshCount.md] reports how many replacements
have happened.

When the problem also has true displacements, the displaced mesh needs no separate treatment: its
coordinates are recomputed from the reference coordinates plus the displacement solution, so it
follows the moved reference mesh on its own.

## Order Within a Time Step id=order

Within the time step, the engine

1. calls the smoother, which updates $\mathbf{d}$, and applies $\mathbf{x} = \mathbf{X}_0 +
   \mathbf{d}$ to the mesh;
2. evaluates every criterion, on the step numbers where
   [!param](/Remeshing/RemeshingAction/check_interval) allows it;
3. hands the mesh to the remesher when a criterion fired;
4. reads the old solution out of the replaced elements, deletes them, rebuilds the equation systems,
   and writes the solution onto the new elements;
5. takes the new snapshot $\mathbf{X}_0$ and returns $\mathbf{d}$ to zero;
6. initializes the stateful material properties of the new elements.

Mesh motion happens on every step; only the criteria are subject to
[!param](/Remeshing/RemeshingAction/check_interval). All of this runs before the solve, so a
postprocessor executing on `TIMESTEP_END` reports the mesh the step was solved on, not the one the
step started from.

[!param](/Remeshing/RemeshingAction/initial_remesh_cycles) runs the engine on the initial condition
as well, before the transient starts. Each cycle evaluates the criteria, hands the mesh to the
remeshers when one fires, and re-projects the initial conditions onto the mesh it produced; the
cycles stop early once no criterion fires. This is how a mesh that disagrees with a target size
field everywhere, such as a background built finer than the field asks for, is brought to the field
before the first time step is solved on it. A restarted or recovered run skips the cycles, since the
checkpointed mesh already carries their result.

A remesher is allowed to reject the patches it was given and change nothing, which the console
reports. The engine then continues as if no criterion had fired.

Only variables holding a single degree of freedom per mesh entity are carried across a replacement,
which covers first order `LAGRANGE` and `CONSTANT MONOMIAL`. Any other variable in the problem ends
the run with an error at the first replacement.

## Stateful Material Properties id=stateful

Raw stateful material property storage is keyed on the elements the surgery deletes, so a
replacement discards it. The one path across a replacement is the
[ProjectedStatefulMaterialStorage](syntax/ProjectedStatefulMaterialStorage/index.md) block, whose
auxiliary variables are carried like any other variable. A problem that has stateful properties
therefore needs two things, and the setup fails with a distinct error when either is missing:

- every stateful property listed in a `[ProjectedStatefulMaterialStorage]` sub-block, and
- `use_interpolated_state = true` on every object that reads the old or older state of one of those
  properties, so that it reads the projected auxiliary variables instead of the raw storage.

!listing test/tests/remeshing/stateful_property_transfer.i block=ProjectedStatefulMaterialStorage

!listing test/tests/remeshing/stateful_property_transfer.i block=Materials

## Adaptivity id=adaptivity

Mesh adaptivity and `[Remeshing]` cannot both modify the mesh in one simulation. Both replace
elements, and their solution transfer paths are incompatible, so an
[Adaptivity](syntax/Adaptivity/index.md) block that names a marker ends the run with an error before
the first solve.

An `[Adaptivity]` block that only defines [Indicators](syntax/Adaptivity/Indicators/index.md)
modifies nothing and is allowed alongside `[Remeshing]`. That is how
[IndicatorThresholdCriterion.md] obtains the field it measures.

## Parallel Execution id=parallel

On a replicated mesh every rank holds the whole mesh, walks it in the same order, and performs the
same surgery, so the copies stay identical without any communication. On a distributed mesh a rank
replaces only the elements it owns, and the engine rebuilds the ghost layer on the reference mesh
and on the displaced mesh alike once the surgery is done. Which elements a rank may reach and how it
cuts its work at the partition seams is the remesher's business; see
[PatchDelaunayRemesher.md#parallel], [TriSplitRemesher.md#parallel] and [TriEdgeRemesher.md#parallel]
for how each of them handles it.

Criteria are reduced over the communicator before they are compared to their threshold, so a
criterion reaches the same verdict on every rank however the mesh is partitioned.

Exodus output cannot be combined with remeshing on a distributed mesh, and the setup ends with an
error naming the output when it is. Exodus output serializes the mesh and renumbers it whenever it
is not contiguously numbered, which is the state a replacement leaves a distributed mesh in, while
$\mathbf{X}_0$ and $\mathbf{d}$ are keyed by node id: the renumbering would leave every key naming
a different node and move the mesh to silently wrong positions. Re-snapshotting the reference
configuration after the renumbering would preserve the geometry, but it would also reset the motion
the criteria threshold on, so the remesh cadence would depend on whether output was turned on; the
combination is refused instead so that output stays observational. Use [Nemesis.md] output, which
writes one file per processor and neither serializes nor renumbers the mesh, or write the
quantities of interest with CSV output. Exodus output stays available on a replicated mesh, whose
node ids are never renumbered.

!syntax list /Remeshing objects=True actions=False subsystems=False

!syntax list /Remeshing objects=False actions=False subsystems=True

!syntax list /Remeshing objects=False actions=True subsystems=False
