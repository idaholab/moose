# AddPeriodicBCAction

!syntax description /BCs/Periodic/AddPeriodicBCAction

Periodic boundary conditions are specified as an object inside the `[Periodic]` sub-block of the `[BCs]` block.
This action adds them to the [Problem](syntax/Problem/index.md).

More information about periodic boundary conditions and their parameters can be found on the
[periodic boundary condition syntax documentation](syntax/BCs/Periodic/index.md).

## Example syntax

In this example, periodic boundary conditions are set on variable `u` in the X and Y axis directions.
The boundaries that are matched on all sides of the system are automatically detected, due to using the
[!param](/BCs/Periodic/AddPeriodicBCAction/auto_direction) parameter.

!listing auto_periodic_bc_test.i block=BCs

!syntax parameters /BCs/Periodic/AddPeriodicBCAction
