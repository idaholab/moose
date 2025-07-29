# Times

Times objects are used to:

- keep track of the times of events during MOOSE-based simulations
- provide a centralized place to input times, so objects can pull in the same times parameters

These tasks may be performed dynamically during the simulation, the user can set the parameter `dynamic_time_sequence` to `true` so that the time sequence will be updated in the begining of each time step. A custom
`initialize()` or `execute()` routine may be implemented for the Times object.

## Combining Times

`Times` from multiple sources may be concatenated using the [ReporterTimes.md].

!syntax list /Times objects=True actions=False subsystems=False

!syntax list /Times objects=False actions=False subsystems=True

!syntax list /Times objects=False actions=True subsystems=False
