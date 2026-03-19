# NumNonlinearIterations

!syntax description /Postprocessors/NumNonlinearIterations

## Description

`NumNonlinearIterations` reports the number of nonlinear iterations in the just-completed
solve.

For situations where the same nonlinear system is solved multiple times per time step, e.g., when a fixed point iteration is performed when using MultiApps, if [!param](/Postprocessors/NumNonlinearIterations/accumulate_over_step) is set to `true`, then the total number of nonlinear iterations per step is returned, rather than just the most recent solve of that system in that time step.

## Example Input Syntax

!listing test/tests/postprocessors/cumulative_value_postprocessor/cumulative_value_postprocessor.i block=Postprocessors

!syntax parameters /Postprocessors/NumNonlinearIterations

!syntax inputs /Postprocessors/NumNonlinearIterations

!syntax children /Postprocessors/NumNonlinearIterations
