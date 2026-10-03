# Mesh Smoothers System

The `[Smoothers]` sub-block of the [Remeshing system](syntax/Remeshing/index.md) holds the object
that moves the mesh between mesh replacements. Exactly one smoother is required when
[!param](/Remeshing/RemeshingAction/mesh_movement) is `true`, and none is accepted when it is not.

A smoother writes the pseudo-displacement $\mathbf{d}$ accumulated since the last replacement, and
the engine then sets every node to $\mathbf{x} = \mathbf{X}_0 + \mathbf{d}$. What is written is the
total motion since the snapshot, not an increment, which is what makes returning $\mathbf{d}$ to
zero at a replacement exact. See [the reference configuration](syntax/Remeshing/index.md#motion).

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing/Smoothers

!syntax list /Remeshing/Smoothers objects=True actions=False subsystems=False

!syntax list /Remeshing/Smoothers objects=False actions=False subsystems=True

!syntax list /Remeshing/Smoothers objects=False actions=True subsystems=False
