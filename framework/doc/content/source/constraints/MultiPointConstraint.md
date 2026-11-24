# MultiPointConstraint

`MultiPointConstraint` is the base class of the constraints that tie a degree of freedom to a
linear combination of other degrees of freedom. It is a [Constraint](syntax/Constraints/index.md)
that assembles no residual and no Jacobian. It adds exact constraints to the `libMesh::DofMap` of
the nonlinear system. libMesh applies them to the element matrices and element vectors that MOOSE
assembles.

This class is not registered, so you cannot use it in an input file. The Solid Mechanics module
builds two objects on it:
[RBE2Constraint](source/constraints/RBE2Constraint.md optional=True) and
[RBE3Constraint](source/constraints/RBE3Constraint.md optional=True).

## Exact constraints

An exact constraint makes one dependent degree of freedom $u_{d}$ a linear function of a set of
independent degrees of freedom $u_{j}$:

\begin{equation}
u_{d} = \sum_{j} c_{dj}\, u_{j} + h_{d} .
\end{equation}

The derived class computes the coefficients $c_{dj}$ from the undisplaced mesh.

Put the exact constraints of a system together as the rectangular matrix $C$ and the vector
$\mathbf{h}$. They map the vector of unconstrained degrees of freedom $\mathbf{u}_{r}$ to the full
vector:

\begin{equation}
\mathbf{u} = C\,\mathbf{u}_{r} + \mathbf{h} .
\end{equation}

Each exact constraint is one row of $C$.

libMesh reduces each element matrix and each element vector with $C$ in
`constrain_element_matrix` and `constrain_element_vector`. MOOSE calls these functions for each
tagged matrix and vector that it assembles. One set of exact constraints reduces the stiffness
matrix, the mass matrix, and the residual:

\begin{equation}
\tilde K = C^{\top} K C, \qquad \tilde M = C^{\top} M C, \qquad
\tilde{\mathbf{R}} = C^{\top} \mathbf{R} .
\end{equation}

The reduction uses only $C$. libMesh uses $\mathbf{h}$ in the constraint equations that it adds
to the nonlinear system, and when it enforces the exact constraints on the solution vector.

The transpose $C^{\top}$ moves a force on a dependent degree of freedom to the independent degrees
of freedom, with the same coefficients. A multipoint constraint adds no stiffness and no mass. It
uses no penalty parameter and no Lagrange multiplier.

libMesh accepts a non-zero right-hand side $\mathbf{h}$ in `DofMap::add_constraint_row()`. The
current objects built on this class all use $\mathbf{h} = \mathbf{0}$. To prescribe a
motion with these objects, apply a [DirichletBC.md] to the independent degrees of freedom. An
eigenvalue problem is homogeneous, so a non-zero $\mathbf{h}$ has no meaning in an eigenvalue
solve.

Each dependent degree of freedom has one exact constraint. If libMesh already constrains a degree
of freedom, for example a hanging node of an adapted mesh, libMesh raises an error. The objects
request this error: they call `add_constraint_row()` with `forbid_constraint_overwrite = true`.

The coefficients describe small motions about the undisplaced mesh, so
`use_displaced_mesh = true` raises an error.

## The exact constraint functor

libMesh accepts one `libMesh::System::Constraint` object for each `System`. libMesh calls this
object through `System::user_constrain()` each time it rebuilds the constraints of the system.
`ExactConstraintFunctor` is this object for a nonlinear system. It adds the exact constraints of
each active constraint whose `usesExactConstraints()` is true. Each `MultiPointConstraint` is such a
constraint, but other constraint families can also use exact constraints.

`NonlinearSystemBase` builds and attaches the functor when it adds the first constraint that uses
exact constraints. This occurs before libMesh distributes the degrees of freedom. If the libMesh
system already has a different constraint object, MOOSE raises an error.

A problem with `displacements` has a displaced copy of the nonlinear system. MOOSE assembles each
object with `use_displaced_mesh = true` through the `Assembly` of this copy. The copy reduces its
element matrices and vectors with the constraints of its own `DofMap`, so MOOSE attaches a second
functor to it. The two systems have the same degree of freedom numbering, so the two functors add
the same exact constraints.

libMesh calls the functor after mesh refinement, coarsening, and repartitioning, and the functor
adds the exact constraints again. The new nodes that refinement adds to a dependent boundary
get their exact constraints at the same time.

## Exact constraints that change

The exact constraints of a system change in two cases:

- A `Control` enables or disables a constraint that uses exact constraints.
- The coefficients of an exact constraint depend on a controllable parameter. Such an object
  overrides `virtual bool exactConstraintsChanged()`. The override compares the parameter with the
  value that the object last used, records the new value, and returns true when they differ.

At the start of each solve, MOOSE compares the active constraints that use exact constraints with
the constraints that it last applied, and asks each of them if its coefficients changed. When any
rank finds a change, MOOSE rebuilds the exact constraints on all ranks. MOOSE does this check only
at the start of a solve, so all residuals and Jacobians of a solve use the same constraints. A
change during a solve takes effect at the next solve.

Rebuilding the exact constraints does not rebuild the sparsity pattern. A rebuild can remove an
exact constraint, add it again, or change its coefficients. It cannot connect degrees of freedom
that the first build did not connect, because that change needs a sparsity rebuild. For example, a
constraint that is disabled in the input file cannot be enabled later.

## Behavior in an eigenvalue solve

In an [EigenProblem.md], libMesh removes each degree of freedom that the `DofMap` constrains from
the stiffness matrix and from the mass matrix. The solve gets the reduced problem
$\tilde K \mathbf{v} = \lambda \tilde M \mathbf{v}$. This problem has no spurious eigenvalues and no
shift from a penalty.
[The eigen tag section of the Constraints syntax page](syntax/Constraints/index.md#eigen-tag) gives
the executioner settings that a constrained `DofMap` needs.

## Deriving a multipoint constraint

`Constraint` declares the exact constraint interface, so any constraint family can use it:

- `usesExactConstraints()` tells the functor and the assembly loops that the object uses exact
  constraints. `MultiPointConstraint` returns true. A family that has exact constraints as one of
  several formulations can return a value that depends on its own parameters.
- `addExactConstraints(libMesh::DofMap &) const` adds the exact constraints. It is pure virtual on
  `MultiPointConstraint`.
- `exactConstraintsChanged()` reports a coefficient change, as the section above describes.
- MOOSE does not add coupling entries to the Jacobian sparsity for a constraint that uses exact
  constraints, whatever its `addCouplingEntriesToJacobian()` returns. libMesh adds the sparsity
  entries that connect a dependent degree of freedom to its independent degrees of freedom.

`MultiPointConstraint` also derives from `Coupleable` as a nodal object, so a derived class can
get its coupled variables directly.

A derived class must implement `addExactConstraints(libMesh::DofMap & dof_map) const` and
`variable()`. `addExactConstraints()` adds each exact constraint with
`dof_map.add_constraint_row(dependent_dof, row, /*forbid_constraint_overwrite=*/true)`.

`addExactConstraints()` is collective. Each rank calls it each time libMesh rebuilds the
constraints. Each rank adds the exact constraints of the dependent nodes that it owns. libMesh then
sends each exact constraint to the other ranks that need it.

The class has four protected helpers. `gatherBoundaryNodes` and `gatherSingleNode` are
collective, so each rank must call them the same number of times and in the same order.
`getConstrainedVars` and `ownedNodes` are local.

- `getConstrainedVars(param_name)` returns the variables of a coupled variable parameter. It raises
  an error when an entry is a constant, or when a variable is not in the nonlinear system of the
  constraint. `Coupleable::getVar()` raises the error for a vector, array, or finite volume
  variable.
- `ownedNodes(boundary)` returns the IDs of the nodes of a node set or side set that this rank
  owns. This rank adds the exact constraints of these nodes.
- `gatherBoundaryNodes(boundary, variables)` returns three items for each node of the boundary:
  the ID, the undisplaced coordinates, and the degree of freedom of each requested variable. The
  rank that owns a node sends it, and all ranks get the full result, sorted by node ID. All ranks
  then compute the same coefficients in the same order. An entry is
  `libMesh::DofObject::invalid_id` when the variable has no degree of freedom at the node.
- `gatherSingleNode(boundary, variables)` does the same gather for a boundary with exactly one
  node. It raises an error when the boundary has a different number of nodes, or when a variable
  has no degree of freedom at the node.

`MultiPointConstraint::validParams()` suppresses `variable` and the tag parameters `vector_tags`,
`matrix_tags`, `extra_vector_tags`, `extra_matrix_tags`, and `absolute_value_vector_tags`. The
object assembles nothing, and it does not necessarily act on a single variable. A derived class
declares its constrained variables with `addRequiredCoupledVar` or `addCoupledVar`, and gets them
with `getConstrainedVars`. All of them must be in one nonlinear system. Its `variable()` returns
one of them.
