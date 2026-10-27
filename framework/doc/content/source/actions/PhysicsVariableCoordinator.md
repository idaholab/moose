# PhysicsVariableCoordinator

This action lets several [Physics](Physics/index.md) objects, for example one per component, share a
single variable defined on the union of their blocks.

A Physics requests such a variable with `PhysicsBase::addSharedSolverVariable()` or
`PhysicsBase::addSharedAuxVariable()` instead of adding it directly. Requests for the same variable
name are merged and their blocks are combined. All requests for a given name must agree on the
family, order, scaling factor and solver system, otherwise an error is reported.

The merged variables are added to the problem on the `flush_shared_physics_variables` task. This task runs after
every Physics has made its requests, and before the initial conditions, materials, kernels,
user objects, boundary conditions and auxiliary kernels that may couple to the variables by name.

The coordinator is created on demand by the first Physics that requests a shared variable, and is not
meant to be declared in an input file.
