# TimeSequenceStepperBase

`TimeSequenceStepperBase` is a base class for time steppers that use a sequence
time values $t_i$ to produce time step sizes.

## Failed solves id=failed_solves

If the solver fails to obtain a converged solution for a given
step, the executioner cuts back the step size and attempts to advance the time
from the previous step using a smaller time step. The time step is cut back by
multiplying the time step by the cutback factor, defaulting to 0.5. If this is successful,
the time stepper will then attempt to use the next time in the sequence,
adjusting the time step to "get back on track".

## Choosing the time step size past the final time value

Suppose that $t_N$ is the maximum time value provided in the sequence. This value
may be less than the simulation end time $t_\text{end}$ given by the
[!param](/Executioner/Transient/end_time) parameter. If this is true and the current simulation
time $t$ is past $t_N$, then by default, there will be a single, final time step
that jumps to the end time:

!equation
\Delta t = t_\text{end} - t_N \,.

However, in many cases this is undesirable, such as when a steady state condition
is used to terminate a transient, in which case an arbitrarily large end time
is specified, leading to a very large time step size. This behavior can be
altered with [!param](/Executioner/TimeSteppers/TimeSequenceStepper/use_last_dt_after_last_t).
If set to `true`, this uses the final time step size in the sequence instead
for all time past $t_N$:

!equation
\Delta t = t_N - t_{N-1} \,.
