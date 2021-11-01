# TimeSequenceStepper

!syntax description /Executioner/TimeStepper/TimeSequenceStepper

If the solver fails to obtain a converged solution for a given
step, the executioner cuts back the step size and attempts to advance the time
from the previous step using a smaller time step. The time step is cut back by
multiplying the time step by the cutback factor, defaulting to 0.5. If this is successful,
the time stepper will then attempt to use the next time in the sequence, adjusting the time step to "get back on track".

## Example input syntax

In this example, the numerical problem is solved at four specified points in time using
a `TimeSequenceStepper`.

!listing test/tests/time_steppers/timesequence_stepper/timesequence.i block=Executioner

!syntax parameters /Executioner/TimeStepper/TimeSequenceStepper

!syntax inputs /Executioner/TimeStepper/TimeSequenceStepper

!syntax children /Executioner/TimeStepper/TimeSequenceStepper
