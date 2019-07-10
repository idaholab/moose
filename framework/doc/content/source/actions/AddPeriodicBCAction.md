# AddPeriodicBCAction

Periodic boundary conditions enforce value constraints on separate boundaries in the mesh.
This capability is useful for modeling quasi-infinite domains and systems with conserved
quantities.

MOOSE has full support for Periodic BCs in

- 1D, 2D, and 3D.
- With mesh adaptivity.
- Can be restricted to specific variables.

Supports arbitrary translation vectors for defining periodicity.

## Automatic Periodic Boundary Detection (recommended)

!listing auto_periodic_bc_test.i block=BCs

## Translation Periodic Boundaries

!listing periodic/periodic_bc_test.i block=BCs

## Function Transform Periodic Boundaries

!listing periodic/trapezoid.i block=Functions BCs

!syntax parameters /BCs/Periodic/AddPeriodicBCAction

!bibtex bibliography
