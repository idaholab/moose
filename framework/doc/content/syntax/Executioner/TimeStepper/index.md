# TimeStepper System

The method used to calculate the size of the time steps is controlled by the
`TimeStepper` block. There are a number of types of TimeStepper available, each
of which controls the time stepping in different ways, including using fixed
time stepping, time stepping based on a function, or adaptive time stepping.

These TimeSteppers can be used separately or used as inputs for [Composition TimeStepper](timesteppers/CompositionDT.md) to generate a composed time step size. If multiple TimeSteppers are used in `TimeStepper` block, the user is required to specify a [!param](/Executioner/Transient/final_time_stepper) to compute the time step size. Below is an example using multiple TimeSteppers input:

!listing test/tests/time_steppers/composition_dt/composition_dt.i block=Executioner

!syntax list /Executioner/TimeStepper objects=True actions=False subsystems=False

!syntax list /Executioner/TimeStepper objects=False actions=False subsystems=True

!syntax list /Executioner/TimeStepper objects=False actions=True subsystems=False
