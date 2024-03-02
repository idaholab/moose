# AddLinearFVKernelAction

!syntax description /LinearFVKernels/AddLinearFVKernelAction

Linear finite volume kernels are specified as objects inside the `[LinearFVKernels]` block.
This action adds them to the [Problem](syntax/Problem/index.md). The main responsibility of
these kernels is to add contributions to the [linear system matrix and right hand side](LinearSystem.md).

More information about linear finite volume kernels can be found on the
[LinearFVKernels syntax documentation](syntax/LinearFVKernels/index.md).

!syntax parameters /LinearFVKernels/AddLinearFVKernelAction
