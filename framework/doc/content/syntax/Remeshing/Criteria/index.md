# Remesh Criteria System

The `[Criteria]` sub-block of the [Remeshing system](syntax/Remeshing/index.md) holds the tests that
decide when the mesh is replaced. At least one criterion is required, and the mesh is replaced when
any of them fires.

A criterion measures the current configuration $\mathbf{x} = \mathbf{X}_0 + \mathbf{d}$, or the
displaced configuration when [!param](/Remeshing/RemeshingAction/displacements) names the
displacement variables of the problem. Each one reduces its measured quantity over the communicator
before comparing it to its threshold, so it reaches the same verdict on every rank. Every criterion
is evaluated on every check, without short-circuiting on the first one that fires.

Criteria are only evaluated on the time steps that
[!param](/Remeshing/RemeshingAction/check_interval) allows; mesh motion is unaffected by it.

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing/Criteria

!syntax list /Remeshing/Criteria objects=True actions=False subsystems=False

!syntax list /Remeshing/Criteria objects=False actions=False subsystems=True

!syntax list /Remeshing/Criteria objects=False actions=True subsystems=False
