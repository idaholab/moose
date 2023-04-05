# AddTimeStepperAction

!syntax description /Executioner/TimeStepper/AddTimeStepperAction

A time stepper is specified as an object inside the `[TimeStepper]` under the `[Executioner]` block
as shown below

!listing test/tests/time_steppers/composition_dt/composition_dt.i block=Executioner

This action add all the time steppers in `[TimeStepper]` block to the transient executioner. The input time steppers can be used in `CompositionDT` to produce a composed time size with given composition rules.


More information about TimeStepper system may be found on the [TimeStepper System syntax documentation](syntax/Executioner/TimeStepper/index.md).

!syntax parameters /Executioner/TimeStepper/AddTimeStepperAction
