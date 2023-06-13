# AuxScalarKernels System

An `AuxiliaryScalarVariable` is to a [ScalarVariable.md] what an [AuxVariable.md] is to a [MooseVariable.md].
It is not the solution of a differential equation and instead can be computed directly by algebraic operations
using an auxiliary scalar kernel.

Creating a custom `AuxScalarKernel` object is done by creating a new C++ object that inherits from
`AuxScalarKernel` and overriding the `computeValue` method.

`AuxScalarKernel` objects, like all `Kernel` objects, must operate on a variable. Thus, there is a required
parameter ("variable") that indicates the variable that the `AuxScalarKernel` object is computing. These
variables are defined in the [AuxVariables](syntax/AuxVariables/index.md) block of the input file, and must be of
family `SCALAR`.

!alert note
For higher order scalar variables, `computeValue` is called multiple times with each order index `_i` for the value of each order.
The definition of `computeValue` may depend on `_i`, as appropriate.

## Execution schedule

Please see the [documentation for field auxiliary kernels (AuxKernels)](syntax/AuxKernels/index.md#execute_on)
which applies identically to `AuxScalarKernels`.

## Examples

`AuxScalarKernels` are essentially used for postprocessing or for decoupling solves.
The examples in the [documentation for field auxiliary kernels (AuxKernels)](syntax/AuxKernels/index.md#example_a)
can conceptually be adapted to `AuxScalarKernels`.

!syntax list /AuxScalarKernels objects=True actions=False subsystems=False

!syntax list /AuxScalarKernels objects=False actions=False subsystems=True

!syntax list /AuxScalarKernels objects=False actions=True subsystems=False
