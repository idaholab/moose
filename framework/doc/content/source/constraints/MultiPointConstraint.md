# MultiPointConstraint

`MultiPointConstraint` is the base class of the constraints that tie a degree of freedom to a
linear combination of other degrees of freedom. It is a [Constraint](syntax/Constraints/index.md)
that assembles neither a residual nor a Jacobian. Instead it adds rows to the `libMesh::DofMap` of
the nonlinear system, and libMesh applies those rows to everything MOOSE assembles.

This class is not registered, so it does not appear in an input file. The objects built on it are
[RBE2Constraint](source/constraints/RBE2Constraint.md optional=True),
[RBE3Constraint](source/constraints/RBE3Constraint.md optional=True) and
[InclinedNoDisplacementConstraint](source/constraints/InclinedNoDisplacementConstraint.md optional=True),
all three in the Solid Mechanics module.

## Constraint rows

A row makes one dependent degree of freedom $u_{d}$ an exact linear function of a set of
independent degrees of freedom $u_{j}$, with coefficients $c_{dj}$ that this class computes once
from the undisplaced mesh:

\begin{equation}
u_{d} = \sum_{j} c_{dj}\, u_{j} .
\end{equation}

Collect the rows of a system into the rectangular matrix $C$, which maps the vector of the
remaining, unconstrained degrees of freedom $\mathbf{u}_{r}$ to the full vector
$\mathbf{u} = C\,\mathbf{u}_{r}$. libMesh reduces every element matrix and every element vector
with $C$ in `constrain_element_matrix` and `constrain_element_vector`, which MOOSE calls for every
tagged matrix and vector it assembles. One set of rows therefore reduces the stiffness matrix, the
mass matrix and the load vector alike:

\begin{equation}
\tilde K = C^{\top} K C, \qquad \tilde M = C^{\top} M C, \qquad
\tilde{\mathbf{f}} = C^{\top} \mathbf{f} .
\end{equation}

The transpose is what makes a row a work-conjugate relation and not only a kinematic one. A force
applied to a dependent degree of freedom arrives on the independent degrees of freedom weighted by
the same coefficients. A multipoint constraint therefore adds no stiffness and no mass of its own,
and it introduces no penalty parameter and no Lagrange multiplier.

The right-hand side of a row is always zero, so a multipoint constraint expresses a homogeneous
relation. Prescribe a motion with a [DirichletBC.md] on the independent degrees of freedom.

A dependent degree of freedom carries exactly one row. A degree of freedom that libMesh already
constrains, such as a hanging node of an adapted mesh, raises an error rather than losing its
existing row.

`use_displaced_mesh = true` raises an error, because the coefficients of a row are built once from
the undisplaced mesh and therefore describe small motions about the reference configuration.

## The constraint hub

libMesh admits one `libMesh::System::Constraint` object per `System` and calls it through
`System::user_constrain()` every time it rebuilds the constraints of that system.
`MultiPointConstraintHub` is that single object. It holds the nonlinear system and asks every
active constraint of that system whose `usesConstraintRows()` is true for its rows. Every
`MultiPointConstraint` answers true, and so does any other constraint that enforces its relation
with rows: a [NodalConstraint.md] or an [EqualValueEmbeddedConstraint.md] with
`formulation = rows` is served by the same hub.

`NonlinearSystemBase` builds a hub and attaches it when the first constraint that uses rows is
added, which happens before libMesh distributes the degrees of freedom. The attachment fails with
an error when another constraint object already occupies that slot on the libMesh system.

A problem with `displacements` carries a displaced copy of the nonlinear system, and MOOSE
assembles every object with `use_displaced_mesh = true` through the `Assembly` of that copy. The
copy reduces its element matrices and vectors with the constraints of its own `DofMap`, so a second
hub is attached to it. The two systems share their degree of freedom numbering, so both hubs add
the same rows.

Because the rows are rebuilt from `user_constrain()`, they come back by themselves after mesh
refinement, coarsening, or repartitioning. No object in MOOSE re-adds a row by hand, and the nodes
that a refinement creates on a dependent boundary receive their rows at the next rebuild.

## Rebuilding rows that a parameter changed

A row provider whose coefficients depend on a controllable parameter overrides
`virtual bool constraintRowsChanged()`, compares the current parameter against the values it last
emitted, refreshes its copy, and answers true. Once per solve, before it sets the initial solution,
`NonlinearSystemBase::preSolve()` asks every active row provider that question, takes the maximum
of the answers across the ranks so that they all agree, and rebuilds the rows of the system when
any provider reports a change. The object never calls the hub itself. A caller that knows the rows
are stale, a `Control` or a `UserObject` for instance, may instead call the public
`NonlinearSystemBase::reinitConstraintRows()`.

The rebuild covers the coefficients of an existing set of rows. The sparsity pattern and the send
list are built once, from the rows of the first build, so a provider may change the weight of a tie
between solves but not which degrees of freedom the tie connects. Changing that is a mesh change.
[LinearNodalConstraint.md], whose `weights` parameter is controllable, is the implementation to
follow.

## Behavior under an eigenvalue solve

Under [EigenProblem.md], every degree of freedom that the `DofMap` constrains is condensed out of
both the stiffness matrix and the mass matrix, so the solve sees the exact reduced problem
$\tilde K \mathbf{v} = \lambda \tilde M \mathbf{v}$, with no spurious eigenvalue and no shift from a
penalty.
[The eigen tag section of the Constraints syntax page](syntax/Constraints/index.md#eigen-tag)
gives the executioner settings that a constrained `DofMap` requires, the sparsity setting that it
does not require, and the behavior of the penalty and Lagrange-multiplier constraint families for
comparison.

## Deriving a multipoint constraint

The row-provider contract lives on `Constraint`, so any constraint family can fulfil it:

- `usesConstraintRows()` tells the hub and the assembly loops that this object is enforced with
  rows. `MultiPointConstraint` answers true for every object built on it; a family that offers rows
  as one of several formulations answers according to its own `formulation` parameter.
- `addConstraintRows(libMesh::DofMap &) const` adds the rows. It is pure on
  `MultiPointConstraint`.
- `constraintRowsChanged()` reports a coefficient change, as described above.
- `addCouplingEntriesToJacobian()` returns the negation of `usesConstraintRows()`, because libMesh
  already expands the sparsity pattern of a constrained degree of freedom to the degrees of freedom
  that constrain it.

`MultiPointConstraint` also derives from `Coupleable` as a nodal object, so a derived class reaches
its coupled variables without deriving from `Coupleable` itself.

A derived class implements one method,
`virtual void addConstraintRows(libMesh::DofMap & dof_map) const`, and adds its rows with
`dof_map.add_constraint_row(dependent_dof, row, /*forbid_constraint_overwrite=*/true)`.

`addConstraintRows()` is collective. Every rank runs it each time libMesh rebuilds the constraints,
and every rank adds the rows of the dependent nodes it has. The class supplies three protected
helpers, and all three are collective as well, so every rank must call each of them the same number
of times and in the same order:

- `localNodes(boundary)` returns the ids of the nodes of a node set or side set that are present on
  this rank. These are the nodes whose rows this rank adds.
- `gatherBoundaryNodes(boundary, var_numbers)` returns the id, the undisplaced coordinates and the
  degree of freedom index of each requested variable for every node of the boundary. The owning
  rank contributes each node, the result is broadcast, and it is sorted by node id, so every rank
  builds the same coefficients in the same order. An entry is `libMesh::DofObject::invalid_id` when
  the variable has no degree of freedom at that node.
- `gatherSingleNode(boundary, var_numbers)` is the same gather for a boundary that must hold exactly
  one node, and it errors when the boundary holds any other number.

Raise an error that only some ranks can detect, such as a missing degree of freedom at a dependent
node, after every rank agrees on the message. An error raised on a subset of the ranks inside a
collective method leaves the other ranks waiting.
[RBE2Constraint](source/constraints/RBE2Constraint.md optional=True) shows this pattern: it
collects a local message, gathers the messages of every rank, and then raises the first one that is
not empty.

`MultiPointConstraint::validParams()` suppresses `variable` and the tagging parameters
`vector_tags`, `matrix_tags`, `extra_vector_tags`, `extra_matrix_tags` and
`absolute_value_vector_tags`, because the object assembles nothing and does not act on a single
variable. A derived class declares the variables it constrains itself, with
`addRequiredCoupledVar` or `addCoupledVar`, and overrides `variable()` to return one of them.
