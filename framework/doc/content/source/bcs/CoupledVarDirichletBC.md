# CoupledVarDirichletBC

!syntax description /BCs/CoupledVarDirichletBC

## Description

`CoupledVarDirichletBC` is a generalization of the `DirichletBC`. Instead of
coupling a single real value through the `value` parameter, a coupled variable
is provided for the `v` parameter.

Parameter [!param](/BCs/CoupledVarDirichletBC/scale_factor) can be used to scale the boundary condition value with a Real value or a Function. By default, it is $1.$ (i.e. no scaling).
Note that [!param](/BCs/CoupledVarDirichletBC/scale_factor) being a Function, it may be used to turn on or off the boundary condition on part of the domain at different points in time. 

## Example Input Syntax

!listing test/tests/bcs/coupled_var_dirichlet/coupled_var_dirichlet.i start=[./right] end=[../] include-end=true

!syntax parameters /BCs/CoupledVarDirichletBC

!syntax inputs /BCs/CoupledVarDirichletBC

!syntax children /BCs/CoupledVarDirichletBC
