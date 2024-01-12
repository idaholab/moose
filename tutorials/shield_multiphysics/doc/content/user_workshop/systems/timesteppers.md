#  [Time Stepper System](Executioner/TimeSteppers/index.md)

A system for suggesting time steps for transient executioners.

!---

!listing adapt_tstep_grow_dtfunc.i block=Executioner

Custom objects are created by inheriting from `TimeStepper` overriding `computeDT()`.

!---

## Built-in TimeSteppers

MOOSE includes many built-in TimeStepper objects:

- `ConstantDT`
- `SolutionTimeAdaptiveDT`
- `IterationAdaptiveDT`
- `FunctionDT`
- `PostprocessorDT`
- `TimeSequenceStepper`

!---

## IterationAdaptiveDT

IterationAdaptiveDT grows or shrinks the time step based on the number of iterations taken to obtain
a converged solution in the last converged step.

!listing adapt_tstep_shrink_init_dt.i block=Executioner

!---

## TimeSequenceStepper

Provide a vector of time points using parameter `time_sequence`, the object simply moves through
these time points.

The $t_{start}$ and $t_{end}$ parameters are automatically added to the sequence.

Only time points satisfying $t_{start} < t <t_{end}$ are considered.

If a solve fails at step $n$ an additional time point $t_{new} = \frac{1}{2}(t_{n+1}+t_n)$ is
inserted and the step is resolved.

!---

## Composing TimeSteppers

+New Feature+: Time steppers can now be composed to follow complex time histories.
By default, the minimum of all the time steps computed by all the time steppers is used!

What steps will be taken, starting at time = 0s?

```bash
[TimeSteppers]
  [constant]
    type = ConstantDT
    dt = 0.2
  []
  [hit_these_times]
    type = TimeSequenceStepper
    time_sequence = '0.5 1 1.5 2.1'
  []
[]
```
