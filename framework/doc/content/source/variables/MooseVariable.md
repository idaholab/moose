# MooseVariable

!syntax description /Variables/MooseVariable

A detailed description of the MOOSE variable system is given in the [base class documentation](MooseVariableBase.md).

Some of the major classes for contributing to the residual and Jacobian equations for spatial
variables are: [Kernels](syntax/Kernels/index.md), [DGKernels](syntax/DGKernels/index.md),
[InterfaceKernels](syntax/InterfaceKernels/index.md), [Boundary Conditions](syntax/BCs/index.md),
and [Constraints](syntax/Constraints/index.md).

For coupling with scalar variables,
then augmentations of the base classes, such as the `KernelScalarBase`
for the `Kernel` class, can be used as described here:
[ScalarKernels/index.md#couple-spatial].

!syntax parameters /Variables/MooseVariable

!syntax inputs /Variables/MooseVariable

!syntax children /Variables/MooseVariable
