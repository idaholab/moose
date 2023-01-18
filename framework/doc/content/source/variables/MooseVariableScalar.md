# MooseVariableScalar

!syntax description /Variables/MooseVariableScalar

A detailed description of the MOOSE variable system is given in the [base class documentation](MooseVariableBase.md).

This variable type is very useful for solving ODEs where the independent
variable is time, e.g. the variable does not have any spatial dependence.

Typically, contribution to the residual and Jacobian equations for scalar
variables are handled using `ScalarKernels`. For coupling with spatial variables,
then augmentations of the base classes, such as the `KernelScalarBase`
for the `Kernel` class, can be used as described here:
[ScalarKernels/index.md#couple-spatial].

!syntax parameters /Variables/MooseVariableScalar

!syntax inputs /Variables/MooseVariableScalar

!syntax children /Variables/MooseVariableScalar
