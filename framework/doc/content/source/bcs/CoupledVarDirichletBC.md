# CoupledVarDirichletBC

!syntax description /BCs/CoupledVarDirichletBC

## Description

`CoupledVarDirichletBC` is a generalization of the `DirichletBC`. Instead of
coupling a single real value through the `value` parameter, a coupled variable
is provided for the `v` parameter.

Parameter $scale_factor$ can be used to scale the boundary condition value with a Real value, a Function or a Variable. By default, it is $1.$ (i.e. no scaling).
Note that $scale_factor$ can be a field variable, so spatially dependent scaling is possible.
This can be used to locally turn the BC on or off.

## Example Input Syntax

!listing test/tests/bcs/coupled_var_dirichlet/coupled_var_dirichlet.i start=[./right] end=[../] include-end=true

!syntax parameters /BCs/CoupledVarDirichletBC

!syntax inputs /BCs/CoupledVarDirichletBC

!syntax children /BCs/CoupledVarDirichletBC
