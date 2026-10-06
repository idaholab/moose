# SimpleTurbinePowerScalarAux

!syntax description /AuxScalarKernels/SimpleTurbinePowerScalarAux

If the [!param](/AuxScalarKernels/SimpleTurbinePowerScalarAux/on) function evaluates to 1 (0 maps to `false`, 1 maps to `true`), the power is
equal to the [!param](/AuxScalarKernels/SimpleTurbinePowerScalarAux/power) function, else it is 0.

Both parameters accept a constant value or a function of time, so the power and on/off state can
be varied during a simulation.

!syntax parameters /AuxScalarKernels/SimpleTurbinePowerScalarAux

!syntax inputs /AuxScalarKernels/SimpleTurbinePowerScalarAux

!syntax children /AuxScalarKernels/SimpleTurbinePowerScalarAux
