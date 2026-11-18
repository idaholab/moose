# NumFailedTimeSteps

!syntax description /Postprocessors/NumFailedTimeSteps

## Description

`NumFailedTimeSteps` reports the number of failed time steps. With `current_time_step_only = true`
it reports only the failed solves of the current time step, which is reset to zero when a time step
converges. Executed on `TIMESTEP_BEGIN`, this is the number of failures before the current attempt,
which [Controls](syntax/Controls/index.md#failed_time_steps) can use to handle a repeated time step
differently from the first attempt.

## Example Input Syntax

!listing test/tests/postprocessors/num_failed_timesteps/failed_timesteps.i block=Postprocessors

!syntax parameters /Postprocessors/NumFailedTimeSteps

!syntax inputs /Postprocessors/NumFailedTimeSteps

!syntax children /Postprocessors/NumFailedTimeSteps
