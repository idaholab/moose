# SteadyExecutor

!syntax description /Executors/SteadyExecutor

## Overview

`SteadyExecutor` sets up the problem in its `init()` step. Its run then does these steps:

1. Output the initial condition.
1. Execute the `TIMESTEP_BEGIN` objects.
1. Run each executor in [!param](/Executors/SteadyExecutor/inner_executors) in order. The run stops
   at the first executor that does not converge.
1. If all inner executors converged, execute the `TIMESTEP_END` objects and output the solution.
1. Execute the `FINAL` objects and output.

The output uses time step 0 for the initial condition and time step 1 for the solution, as the
[Steady.md] executioner does. `SteadyExecutor` does no mesh adaptivity and does not execute
MultiApps.

## Example input syntax

!listing test/tests/executors/steady/steady.i block=Executors

!syntax parameters /Executors/SteadyExecutor

!syntax inputs /Executors/SteadyExecutor

!syntax children /Executors/SteadyExecutor
