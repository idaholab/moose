# TimeSequenceStepper

This time stepper derives from [TimeSequenceStepperBase.md] and provides the
sequence of time values from a user-specified list, given by
[!param](/Executioner/TimeSteppers/TimeSequenceStepper/time_sequence).

## Example input syntax

In this example, the numerical problem is solved at four specified points in time using
a `TimeSequenceStepper`.

!listing test/tests/time_steppers/timesequence_stepper/timesequence.i block=Executioner

!syntax parameters /Executioner/TimeSteppers/TimeSequenceStepper

!syntax inputs /Executioner/TimeSteppers/TimeSequenceStepper

!syntax children /Executioner/TimeSteppers/TimeSequenceStepper
