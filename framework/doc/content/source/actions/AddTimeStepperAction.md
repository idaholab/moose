# AddTimeStepperAction

!syntax description /Executioner/TimeSteppers/AddTimeStepperAction

This action handles both `[TimeStepper]` and `[TimeSteppers]` blocks. It supports the use of the `[TimeStepper]` block where only one time stepper object can be specified and the use of `[TimeSteppers]` block where multiple time stepper objects can be specified to compose a time step size with given composition rules. If multiple time steppers(>1) are provided in `[TimeSteppers]` block, a time stepper object named `CompositionDT` will be added to the Time Stepper System which produces the composed time step size with input time steppers.


More information about Time Stepper System and CompositionDT time stepper may be found on the [Time Stepper System syntax documentation](syntax/Executioner/TimeSteppers/index.md).


