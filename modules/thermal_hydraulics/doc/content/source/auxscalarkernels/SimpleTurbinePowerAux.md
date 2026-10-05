# SimpleTurbinePowerScalarAux

!syntax description /AuxScalarKernels/SimpleTurbinePowerScalarAux

If the [!param](/AuxScalarKernels/SimpleTurbinePowerScalarAux/on) parameter is equal to 1 (0 maps to `false`, 1 maps to `true`), the power is
equal to [!param](/AuxScalarKernels/SimpleTurbinePowerScalarAux/value), else it is 0.

The [!param](/AuxScalarKernels/SimpleTurbinePowerScalarAux/on) parameter is controllable,
meaning that its value can be changed dynamically during a simulation using the [Controls system](syntax/Controls/index.md).

!syntax parameters /AuxScalarKernels/SimpleTurbinePowerScalarAux

!syntax inputs /AuxScalarKernels/SimpleTurbinePowerScalarAux

!syntax children /AuxScalarKernels/SimpleTurbinePowerScalarAux
