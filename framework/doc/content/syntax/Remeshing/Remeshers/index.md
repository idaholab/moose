# Remeshers System

The `[Remeshers]` sub-block of the [Remeshing system](syntax/Remeshing/index.md) holds the objects
that replace elements of the reference mesh when a criterion fires. At least one remesher is
required; several are applied in turn, each seeing the mesh the one before it left.

A remesher operates on the undisplaced mesh, whose current coordinates are
$\mathbf{x} = \mathbf{X}_0 + \mathbf{d}$, never on the displaced coordinates: with true
displacements the displacement variables are interpolated onto
the new elements like any other variable, which keeps the description total Lagrangian. It reports
which elements it replaced and where on the old mesh the new entities take their values from, and
the engine reads the old solution through that report before deleting anything.

A remesher may decline to change the mesh, which the console reports and the engine treats as if no
criterion had fired.

!listing test/tests/remeshing/ale_remesh_reset.i block=Remeshing/Remeshers

!syntax list /Remeshing/Remeshers objects=True actions=False subsystems=False

!syntax list /Remeshing/Remeshers objects=False actions=False subsystems=True

!syntax list /Remeshing/Remeshers objects=False actions=True subsystems=False
