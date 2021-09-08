# AddScalarKernelAction

!syntax description /ScalarKernels/AddScalarKernelAction

This action handles both `ScalarKernels` and `AuxScalarKernels`.

Kernels for scalar variables are specified as an object inside the `[ScalarKernels]` block,
while auxiliary kernels for scalar variables are specified as an object inside the `[AuxScalarKernels]` block.
This [MooseObjectAction.md] adds them to the [Problem](syntax/Problem/index.md).

More information about `ScalarKernels` may be found on the
[ScalarKernels syntax documentation](syntax/ScalarKernels/index.md).

!syntax parameters /ScalarKernels/AddScalarKernelAction
