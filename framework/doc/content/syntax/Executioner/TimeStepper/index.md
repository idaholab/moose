# TimeStepper System

The method calculates the size of the time steps using either the `[TimeStepper]` block or the `[TimeSteppers]` block. The `[TimeStepper]` block takes only one time stepper while `[TimeSteppers]` block keeps the features of `[TimeStepper]` and also supports the composed time step size with multiple time steps inputs.

!alert warning
This page and the `[TimeStepper]` block will be deprecated soon in the future.

When more than one time steppers are provided, the time stepper system will add a [Composition TimeStepper](timesteppers/CompositionDT.md) as the final time stepper of the transient simulation. The [Composition TimeStepper](timesteppers/CompositionDT.md) takes all input time steppers except the ones used for [!param](/Executioner/TimeSteppers/lower_bound) and compute the minimum time step size within [!param](/Executioner/TimeSteppers/lower_bound) as the output time step size. There are a number of types of TimeStepper available. They control the time stepping in different ways, including using fixed time stepping, time stepping based on a function, or adaptive time stepping.

The time stepper system is controllable via [Controls](syntax/Controls/index.md) block. The user can turn on/off the time steppers to control the usage of time steppers like make time stepper(s) only active at certain time period.

## Example input syntax

!listing test/tests/time_steppers/time_stepper_system/multiple_timesteppers.i block=Executioner

!listing test/tests/time_steppers/time_stepper_system/active_timesteppers.i  block=Executioner Controls

!syntax list /Executioner/TimeStepper objects=True actions=False subsystems=False

!syntax list /Executioner/TimeStepper objects=False actions=False subsystems=True

!syntax list /Executioner/TimeStepper objects=False actions=True subsystems=False

!syntax parameters /Executioner/TimeStepper
