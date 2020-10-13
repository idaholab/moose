# CoupledVarNeumannBC

!syntax description /BCs/CoupledVarNeumannBC

## Description

`CoupledVarNeumannBC` is a generalization of the `NeumannBC`. Instead of
coupling a single real value through the `value` parameter, a coupled variable
is provided for the `v` parameter.

Parameter $scale$ can be used to scale the boundary condition value. By default, it is $1.$ (i.e. no scaling).
Note that $scale$ is actually a field variable, so spatially dependent scaling is possible.
This can be used to locally turn the BC on or off.

## Example Input Syntax

!listing test/tests/bcs/coupled_var_neumann/coupled_var_neumann.i start=[./right] end=[../] include-end=true

!syntax parameters /BCs/CoupledVarNeumannBC

!syntax inputs /BCs/CoupledVarNeumannBC

!syntax children /BCs/CoupledVarNeumannBC
