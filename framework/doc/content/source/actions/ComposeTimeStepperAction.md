# ComposeTimeStepperAction

!syntax description /Executioner/TimeSteppers/ComposeTimeStepperAction

This action specifies the [CompositionDT.md] object when multiple time steppers are provided in the `[TimeSteppers]` block. If there are more than one time steppers provided, it will add a CompositionDT in the time stepper system and make the CompositionDT as the final time stepper to use in the transient simulations. An optional parameter [!param](/Executioner/TimeSteppers/ComposeTimeStepperAction/lower_bound) is provided to set a lower bound for the computed time step size. It can be specified with any time stepper(s) in the `[TimeSteppers]` block.

An example of using a growing lower bound to limit the time step size as:

!listing test/tests/time_steppers/time_stepper_system/lower_bound.i block=Executioner

More information about Time Stepper System and [CompositionDT.md] time stepper may be found on the [Time Stepper System syntax documentation](syntax/Executioner/TimeSteppers/index.md).


