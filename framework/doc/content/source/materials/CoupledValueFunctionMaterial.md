# CoupledValueFunctionMaterial

!syntax description /Materials/CoupledValueFunctionMaterial

CoupledValueFunctionMaterial evaluates MOOSE Functions with a set of up to
four coupled variable values `v` as its input parameters. The coupled variable
values are substituted for the `x`,`y`,`z`, and `t` function variables in that
order.

With the [!param](/Materials/CoupledValueFunctionMaterial/parameter_order) a
custom mapping from coupled variable to function parameters can be supplied.

!syntax parameters /Materials/CoupledValueFunctionMaterial

!syntax inputs /Materials/CoupledValueFunctionMaterial

!syntax children /Materials/CoupledValueFunctionMaterial
