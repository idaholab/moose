# ConstantDT

!syntax description /Executioner/TimeStepper/ConstantDT

## Description

The `ConstantDT` TimeStepper simply takes a constant time step size
throughout the analysis, unless the solver fails to converge on an iteration.

`ConstantDT` begins the analysis taking the step specified by the user with the
`dt` parameter. If the solver fails to obtain a converged solution for a given
step, the executioner cuts back the step size and attempts to advance the time
from the previous step using a smaller time step. The time step is cut back by
multiplying the time step by 0.5.

If the solution with the cut-back time step is still unsuccessful, the time
step size is repeatedly cut back until a successful solution is obtained. The
user can specify a minimum time step through the `dtmin` parameter in the
`Executioner` block. If the time step must be cut back below the minimum size
without obtaining a solution, the problem exits with an error. If the time step
is cut back using `ConstantDT`, that cut-back step size will be used for the
remainder of the the analysis.

## Example Input Syntax

!listing test/tests/misc/check_error/wrong_displacement_order.i block=Executioner

!syntax parameters /Executioner/TimeStepper/ConstantDT

!syntax inputs /Executioner/TimeStepper/ConstantDT

!syntax children /Executioner/TimeStepper/ConstantDT
