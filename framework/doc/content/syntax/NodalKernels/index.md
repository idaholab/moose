# NodalKernels System

Nodal kernels are used to solve equations that strictly belong on a node.
Some examples include:

- solving ordinary differential equations at every node. The following `NodalKernels` were implemented for that purpose:
  - [ConstantRate.md]
  - [TimeDerivativeNodalKernel.md]
  - [UserForcingFunctionNodalKernel.md]

- bounding a nodal variable's value at nodes. The following `NodalKernels` were implemented for that purpose:
  - [LowerBoundNodalKernel.md]
  - [UpperBoundNodalKernel.md]

- assigning mass to nodes instead of elements, as performed in the Tensor Mechanics module
  (see [NodalGravity](nodalkernels/NodalGravity.md optional=True) for example)


!alert note
Even though we are not using an elemental quadrature in nodal kernels, [Variables](syntax/Variables/index.md) values
should still be accessed at the index `_qp` (=0) for consistency with other kernels' code.

Nodal kernels may be block and boundary restricted. Naturally for boundary restriction, the nodal kernel is only defined on the
nodes that are part of a boundary. For block restriction, all nodes that are part of elements in a block are considered.

!syntax list /NodalKernels objects=True actions=False subsystems=False

!syntax list /NodalKernels objects=False actions=False subsystems=True

!syntax list /NodalKernels objects=False actions=True subsystems=False
