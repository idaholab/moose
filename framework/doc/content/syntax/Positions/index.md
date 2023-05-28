# Positions

Positions are used to keep track of the locations of objects during MOOSE-based simulations.

By default, they are updated when the mesh changes and on every execution. The execution schedule
is by default very limited, but may be expanded using the `execute_on` parameter of every `Positions`
object.

`Positions` support initialization from another `Positions` object. The name of the initialization
object should be specified using the [!param](/Positions/InputPositions/initial_positions) parameter.

## Combining positions

`Positions` from multiple sources may be concatenated using the [ReporterPositions.md].

## Example uses

`Positions` may be used to spawn subapps of a `MultiApp` at various locations, using the
[!param](/MultiApps/FullSolveMultiApp/positions_objects) parameter. The positions
of the subapps will be updated with the `Positions`.

!alert warning
The number of `Positions` should currently stay constant during the simulation.

!syntax list /Positions objects=True actions=False subsystems=False

!syntax list /Positions objects=False actions=False subsystems=True

!syntax list /Positions objects=False actions=True subsystems=False
