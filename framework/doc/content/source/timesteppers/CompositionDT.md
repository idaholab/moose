# CompositionDT

!syntax description /Executioner/TimeSteppers/CompositionDT

## Description

The `CompositionDT` TimeStepper takes multiple time steppers as input and computes the minimum time step size among all time steppers as output. If any time sequence stepper(s) is supplied for input, CompositionDT will compare the computed minimum time step size with the one needs to hit the time point, then select the smaller value as output. An optional parameter [!param](/Executioner/TimeSteppers/CompositionDT/lower_bound)is provided to set a lower bound for the computed time step size, and enable growth driven by a single time stepper.

The composition rules are listed with priority rank:
1. The time points from time sequence stepper(s) **must** be hit;
2. The time step size can not go below the [!param](/Executioner/TimeSteppers/CompositionDT/lower_bound);
3. Take the minimum time step value of all input time steppers.

An example of using multiple time steppers:

!listing test/tests/time_steppers/time_stepper_system/multiple_timesteppers.i block=Executioner

!syntax parameters /Executioner/TimeSteppers/CompositionDT

!syntax inputs /Executioner/TimeSteppers/CompositionDT

!syntax children /Executioner/TimeSteppers/CompositionDT
