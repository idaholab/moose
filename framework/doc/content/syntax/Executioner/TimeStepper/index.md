# TimeStepper System

The method used to calculate the size of the time steps is controlled by the
`TimeStepper` block. There are a number of types of TimeStepper available, each
of which controls the time stepping in different ways, including using fixed
time stepping, time stepping based on a function, or adaptive time stepping.

!syntax list /Executioner/TimeStepper objects=True actions=False subsystems=False

!syntax list /Executioner/TimeStepper objects=False actions=False subsystems=True

!syntax list /Executioner/TimeStepper objects=False actions=True subsystems=False
