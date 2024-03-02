# AddLinearFVBCAction

!syntax description /LinearFVBCs/AddLinearFVBCAction

Linear finite volume boundary conditions are specified as objects inside the `[LinearFVBCs]` block.
This action adds them to the [Problem](syntax/Problem/index.md). The main responsibility of these boundary conditions is to compute contributions to the [linear system matrix and right hand side](LinearSystem.md),
which are then used by [Linear FV kernels](syntax/LinearFVKernels/index.md).

More information about linear finite volume boundary conditions can be found on the
[Linear FVBCs syntax page](syntax/LinearFVBCs/index.md).

!syntax parameters /LinearFVBCs/AddLinearFVBCAction
