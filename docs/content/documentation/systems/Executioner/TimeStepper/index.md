# TimeStepper System

The method used to calculate the size of the time steps is controlled by the
`TimeStepper` block. There are a number of types of `TimeStepper` available.
These permit the time step to be
controlled directly by providing either a single fixed time step to take throughout the analysis,
by providing the time step as a function of time, or by using adaptive timestepping algorithm can
be used to modify the time step based on the difficulty of the iterative solution, as quantified by
the numbers of linear and nonlinear iterations required to drive the residual below the tolerance
required for convergence.

!syntax objects /Executioner/TimeStepper

!syntax subsystems /Executioner/TimeStepper

!syntax actions /Executioner/TimeStepper
