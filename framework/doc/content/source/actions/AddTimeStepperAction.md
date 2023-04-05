# AddTimeStepperAction

!syntax description /Executioner/TimeStepper/AddTimeStepperAction

A `TimeStepper` is specified as an object inside the `[TimeStepper]` block with the `[Executioner]` block
as shown below

!listing test/tests/time_steppers/composition_dt/composition_dt.i block=Executioner

This action add all the `TimeStepper`s in `[TimeStepper]` block to the transient executioner. The input `TimeStepper`s can be used in 'CompositionDT' to produce a composed time size with given composition rules.


More information about TimeStepper system may be found on the [TimeStepperSystem syntax documentation](syntax/Executioner/TimeStepper/index.md).

!syntax parameters /Executioner/TimeStepper/AddTimeStepperAction
