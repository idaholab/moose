# CoupledVarDirichletBC

!syntax description /BCs/CoupledVarDirichletBC

## Description

`CoupledVarDirichletBC` is a generalization of the `DirichletBC`. Instead of
coupling a single real value through the `value` parameter, a coupled variable
is provided for the `v` parameter.

Parameter $scale_factor$ can be used to scale the boundary condition value, either with a Real value or a Function. By default, it is $1.$ (i.e. no scaling).

## Example Input Syntax

!listing test/tests/bcs/coupled_var_dirichlet/coupled_var_dirichlet.i start=[./right] end=[../] include-end=true

!syntax parameters /BCs/CoupledVarDirichletBC

!syntax inputs /BCs/CoupledVarDirichletBC

!syntax children /BCs/CoupledVarDirichletBC
