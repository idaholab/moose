# SetupTimeStepperAction

!syntax description /Executioner/TimeStepper/SetupTimeStepperAction

A `TimeStepper` is specified as an object inside the `[TimeStepper]` block with the `[Executioner]` block
as shown below

!listing test/tests/time_steppers/constant_dt/constant_dt.i block=Executioner

This action sets the `TimeStepper` to the transient executioner.

More information about TimeSteppers may be found on the
[TimeStepper syntax documentation](syntax/Executioner/TimeStepper/index.md).

!syntax parameters /Executioner/TimeStepper/SetupTimeStepperAction
