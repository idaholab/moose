# PhysicsVariableCoordinator

This action lets several [Physics](Physics/index.md) objects, for example one per component, share a
single variable defined on the union of their blocks.

A Physics requests such a variable with `PhysicsBase::addSharedSolverVariable()` or
`PhysicsBase::addSharedAuxVariable()` instead of adding it directly, passing the variable type and
its parameters. Requests for the same variable name are merged and their blocks are combined. All
requests for a given name must agree on the variable type and on every parameter other than `block`,
otherwise an error is reported.

A finite volume variable may only be shared across two adjacent subdomains if one of the requesting
Physics is defined on both. Otherwise, the flux kernels of each Physics would treat the faces
between the subdomains as boundaries of their own blocks, so no flux would cross them even though the
variable is defined on both sides, and an error is reported. Finite volume Physics on subdomains that
do not touch, for example components connected through junctions, may share a variable.

The merged variables are added to the problem on the `flush_shared_physics_variables` task. This task runs after
every Physics has made its requests, and before the initial conditions, materials, kernels,
user objects, boundary conditions and auxiliary kernels that may couple to the variables by name.

The coordinator is created on demand by the first Physics that requests a shared variable, and is not
meant to be declared in an input file.
