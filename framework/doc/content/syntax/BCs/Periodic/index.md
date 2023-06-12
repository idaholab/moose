# Periodic System

Periodic boundary conditions enforce value constraints on separate boundaries in the mesh.
This capability is useful for modeling quasi-infinite domains and systems with conserved
quantities.

MOOSE has full support for Periodic BCs in

- 1D, 2D, and 3D.
- With mesh adaptivity.
- Can be restricted to specific variables.

Arbitrary translation vectors and arbitrary mapping transformations between boundaries are also supported for defining periodicity.

## Automatic Periodic Boundary Detection (recommended)

In this example, periodic boundary conditions are set on variable `u` in the X and Y axis directions.
The boundaries that are matched on all sides of the system are automatically detected, due to using the
[!param](/BCs/Periodic/AddPeriodicBCAction/auto_direction) parameter.

!alert note
The automatic detection of the boundaries to match is only available in the X, Y and Z axis directions.

!listing auto_periodic_bc_test.i block=BCs

## Translation Periodic Boundaries

In this example, periodic boundary conditions are set on variable `u` in the X and Y axis directions, using
the exact distance for the translation needed to go from one periodic boundary to the other.
The boundaries are also explicitly specified using the [!param](/BCs/Periodic/AddPeriodicBCAction/primary)
and [!param](/BCs/Periodic/AddPeriodicBCAction/secondary) parameters.

!listing periodic/periodic_bc_test.i block=BCs

## Function Transform Periodic Boundaries

In this example, periodic boundary conditions are set on variable `u` in arbitrary non-matching directions, using
the mapping functions to go from one boundary to the other. The transformation to go from the primary to the secondary
is given by a [Function](syntax/Functions/index.md) using the [!param](/BCs/Periodic/AddPeriodicBCAction/transform_func) parameter,
and in the other direction the [!param](/BCs/Periodic/AddPeriodicBCAction/inv_transform_func) parameter.
The boundaries are also explicitly specified using the [!param](/BCs/Periodic/AddPeriodicBCAction/primary)
and [!param](/BCs/Periodic/AddPeriodicBCAction/secondary) parameters.

!listing periodic/trapezoid.i block=Functions BCs

!syntax list /BCs/Periodic objects=True actions=False subsystems=False

!syntax list /BCs/Periodic objects=False actions=False subsystems=True

!syntax list /BCs/Periodic objects=False actions=True subsystems=False
