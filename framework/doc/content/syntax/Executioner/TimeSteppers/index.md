# TimeStepper System

This system is in charge of determining the time steps in a transient simulation using either the `[TimeStepper]` block or the `[TimeSteppers]` block. The `[TimeStepper]` block takes only one time stepper while `[TimeSteppers]` block keeps the features of `[TimeStepper]` and also supports the composed time step size with multiple time steps inputs.

!alert warning
The `[TimeStepper]` block will be deprecated in the future.

When more than one time steppers are provided, the time stepper system will add a [Composition TimeStepper](timesteppers/CompositionDT.md) as the final time stepper of the transient simulation. The [Composition TimeStepper](timesteppers/CompositionDT.md) takes all input time steppers except the ones used for [!param](/Executioner/TimeSteppers/lower_bound) and compute the maximum time step size within [!param](/Executioner/TimeSteppers/lower_bound) as the output time step size. There are a number of types of TimeStepper available. They control the time stepping in different ways, including using [fixed time stepping](ConstantDT.md), [time stepping based on a function](FunctionDT.md), or [adaptive time stepping](IterationAdaptiveDT.md).

The time stepper system is controllable via [Controls](syntax/Controls/index.md) block. The user can turn on/off the time steppers to control the usage of time steppers like make time stepper(s) only active [at certain time period](TimePeriod.md).


## Example input syntax

Below are two examples for multiple time steppers input and the control feature of time stepper system:

This example shows the use of multiple time steppers to compose a final time step size. The time stepper system will always choose the minimum value of all input time step.

!listing test/tests/time_steppers/time_stepper_system/multiple_timesteppers.i block=Executioner

This example shows the control feature of the time stepper system. The [TimePeriod.md] control is used to specify a time period for only `ConstDT1` to be active.

!listing test/tests/time_steppers/time_stepper_system/active_timesteppers.i  block=Executioner Controls

## The difference between `[TimeSteppers]`and `[TimeStepper]` input file

!alert warning
The `[TimeSteppers]` block requires one hierarchy of block for input time stepper(s).

Below are two examples show the differences in the block hierarchy between `[TimeSteppers]` and `[TimeStepper]`:

An input with `[TimeStepper]` block:

```
[Executioner]
  type = Transient
  [TimeStepper]
     type = TimeSequenceStepper
     time_sequence  = '0 43200 86400 172800 432000 864000'
  []
  start_time = 0.0
  end_time = 864000
[]
```

An input with `[TimeSteppers]` block:

```
[Executioner]
  type = Transient
  [TimeSteppers]
    [my_timestepper]
     type = TimeSequenceStepper
     time_sequence  = '0 43200 86400 172800 432000 864000'
    []
  []
  start_time = 0.0
  end_time = 864000
[]
```



!syntax list /Executioner/TimeSteppers objects=True actions=False subsystems=False

!syntax list /Executioner/TimeSteppers objects=False actions=False subsystems=True

!syntax list /Executioner/TimeSteppers objects=False actions=True subsystems=False

!syntax parameters /Executioner/TimeSteppers
