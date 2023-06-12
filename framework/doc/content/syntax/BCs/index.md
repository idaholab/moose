# BCs System

The `BCs` system is used to impose boundary conditions in a finite element problem. Each `BC` object
sets the boundary condition for a single variable/equation, though multiple boundaries may be specified at once.

!alert note
See [FVBCs](syntax/FVBCs/index.md) for boundary conditions of finite volume problems.

Some of the main types of boundary conditions are:
- [Dirichlet Boundary conditions](DirichletBC.md) to set the value of a variable on a boundary
- [Neumann Boundary conditions](NeumannBC.md) to set a flux for the equation corresponding to the variable. Depending on the
  equation being solved, this can be equivalent to setting the value of the derivative of the variable on the boundary
- Robin boundary conditions to solve an equation tying both the variable value and its derivatives on the boundary

In MOOSE, boundary conditions are split in two categories: `NodalBC`s and `IntegratedBC`s. Similar to kernels,
`computeQpResidual/Jacobian` are the main routine to implement for most boundary conditions. [Automatic differentiation](automatic_differentiation/index.md)
is implemented for boundary conditions. `AD` BCs do not require implementing `computeQpJacobian`.

## Nodal boundary conditions

Nodal boundary conditions are applied on nodes in the mesh. Nodal boundary conditions may only be applied on variables
that have degrees of freedom at nodes, such as Lagrange variables.

We show below code snippets for the `DirichletBCBase`. This boundary condition sets the value of a variable on
a node.

If the value is preset, the `computeQpValue` is called instead of the `computeQpResidual`. This value is then directly placed in the solution vector.

!listing src/bcs/DirichletBC.C start=Real end=} include-end=true

If the value is not preset, the `computeQpResidual` routine is called. The residual of a non-preset `DirichletBC` is simply the difference between the desired value and the current variable value.

!listing src/bcs/DirichletBCBase.C start=Real end=} include-end=true

!alert note
`_qp`, the index over quadrature points, is simply 0 on nodes. We use this indexing in order to keep consistent user interfaces.

## Integrated boundary conditions

Integrated boundary conditions are applied on sides in the mesh. A local quadrature-based integration is performed to compute
the residual and Jacobian contribution of the boundary condition.

In the following snippet, we show the definition of the contribution to the residual of a [FunctionNeumannBC.md].
The `computeQpResidual` simply returns the value of the flux.

!listing src/bcs/FunctionNeumannBC.C start=Real end=} include-end=true

!syntax list /BCs objects=True actions=False subsystems=False

!syntax list /BCs objects=False actions=False subsystems=True

!syntax list /BCs objects=False actions=True subsystems=False
