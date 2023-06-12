# MooseVariableFE

!syntax description /Variables/MooseVariable

A `MooseVariableFE` is a derived class of a `MooseVariableField`. It is a finite element field variable.
Depending on the finite element family (chosen with [!param](/Variables/MooseVariable/family) parameter)
or the [!param](/Variables/MooseVariable/components) parameter,
it can be a regular, vector or array variable.

A `MooseVariableFE` can be a nonlinear variable, in which case it should be created in the [`[Variables]`](syntax/Variables/index.md)
block, or an auxiliary variable, in which case it should be created in the [`[AuxVariables]`](syntax/AuxVariables/index.md) block.

A `MooseVariableFE` is a [Functor](syntax/Functors/index.md). As such it may be specified in the `functor` parameter of
numerous polyvalent objects.

!syntax parameters /Variables/MooseVariable

!syntax inputs /Variables/MooseVariable

!syntax children /Variables/MooseVariable
