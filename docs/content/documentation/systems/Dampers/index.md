# Dampers System

Dampers are used to decrease the attempted change to the solution with each nonlinear step.
This can be useful in preventing the solver from changing the solution dramatically from one
step to the next. This may prevent, for example, the solver from attempting to evaluate negative
temperatures.

!syntax objects /Dampers

!syntax subsystems /Dampers

!syntax actions /Dampers
