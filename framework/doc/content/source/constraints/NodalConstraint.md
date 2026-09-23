# NodalConstraint

`NodalConstraint` is the base class of the constraints that tie the value of a variable at a
secondary node to a linear combination of its values at a set of primary nodes. It is not
registered, so it never names a `type` in an input file. The framework objects built on it are
[LinearNodalConstraint.md], which reads the primary nodes and their weights from the input, and
[EqualValueBoundaryConstraint.md], which ties every node of a boundary to a single primary node.
[EqualValueEmbeddedConstraint.md] ties a node to the element that contains it rather than to other
nodes, so it derives from `NodeElemConstraint` instead, but it offers the same three formulations.

## The tie

Every object in this family enforces one relation per secondary node $s$:

!equation id=nodal-tie
u_s = \sum_{j} w_j\, u_{p_j}

| symbol | supplied by |
| --- | --- |
| $u_s$ | the secondary variable at secondary node $s$, one relation per secondary node. The secondary variable is `variable`, or `variable_secondary` when the two sides carry different variables |
| $u_{p_j}$ | `variable` at primary node $j$ |
| $w_j$ | the weight of primary node $j$: `weights` on [LinearNodalConstraint.md], and 1 for an equal value tie with a single primary node |

`formulation` selects how [nodal-tie] is enforced. The three values enforce the same relation but
differ in how exactly they hold it and in which parts of the assembly they reach. `penalty` is the
default, so an input that does not set `formulation` keeps the behavior it had before `rows`
existed.

## Penalty and kinematic

`penalty` adds, for each primary-secondary pair, the term $\alpha\,(u_s / n_p - w_j u_{p_j})$ to
the equation of the secondary node and the equal and opposite term to the equation of primary node
$j$, where $\alpha$ is `penalty` and $n_p$ is the number of primary nodes. The tie then holds only
to $O(1/\alpha)$, and $\alpha$ has to be chosen against the scaling of the variable: too small and
the tie leaks, too large and the solve conditions badly.

`kinematic` moves the residual that the secondary node carries onto the primary nodes with the same
weights and leaves the penalty term as the whole secondary equation. The tie then holds to the
nonlinear solver tolerance rather than to $O(1/\alpha)$, but it is still a residual, so `penalty`
is still required and still enters the Jacobian.

Both formulations assemble a residual and a Jacobian, and both ask for the implicit geometric
coupling entries that tie the secondary and primary rows of the sparsity pattern together.

## Degree of freedom constraint rows id=rows

`formulation = rows` assembles no residual and no Jacobian. Instead the object hands libMesh one
`libMesh::DofConstraintRow` per secondary node, whose entries are the primary degrees of freedom
and the weights $w_j$ and whose right-hand side is zero, exactly [nodal-tie]. Collecting those rows
into the matrix $C$ that maps the remaining, unconstrained degrees of freedom $\mathbf{u}_r$ to the
full vector $\mathbf{u} = C \mathbf{u}_r$, libMesh reduces every element matrix and every element
vector that MOOSE assembles:

\begin{equation}
\tilde K = C^{\top} K C, \qquad \tilde M = C^{\top} M C, \qquad
\tilde{\mathbf{f}} = C^{\top} \mathbf{f} .
\end{equation}

The tie is therefore exact, work-conjugate, and free of any penalty parameter, and it reaches every
matrix MOOSE assembles rather than the system matrix alone. `penalty` may be omitted under `rows`;
it is required under the other two formulations and ignored under this one. The same mechanism is
described in more detail on [MultiPointConstraint.md], which is the constraint family built on rows
alone.

On `linear_nodal_constraint_tie_error.i` below, which ties node 4 to ten times node 0 with
`penalty = 1e5`, the measured tie error $u_s - 10\,u_p$ is exactly 0 under `rows`,
$-4.0 \times 10^{-14}$ under `kinematic`, and $-6.4 \times 10^{-6}$ under `penalty`; the three gold
files in that directory hold those values. `rows` and `kinematic` reach the same converged solution
and take the same number of Newton steps on that problem, so `rows` buys exactness and eigen
correctness rather than speed. Where it does change the solve is where `kinematic` fails: the
kinematic formulation does not converge on
`test/tests/constraints/nodal_constraint/linear_nodal_constraint_different_variables.i` or on
`test/tests/constraints/equal_value_boundary_constraint/adaptivity.i`, and `rows` converges on
both.

### Under an eigenvalue solve

A penalty or kinematic nodal constraint is not enforced at all under
[Executioner/type = Eigenvalue](EigenProblem.md), because its Jacobian contribution is gated on the
system matrix tag and an eigen assembly carries only the eigen or the noneigen tag. `rows` is the
fix: the constrained degrees of freedom are condensed out of both matrices, so the solve sees the
exact reduced problem $\tilde K \mathbf{v} = \lambda \tilde M \mathbf{v}$.
[The eigen tag section of the Constraints syntax page](syntax/Constraints/index.md#eigen-tag)
gives the executioner settings a constrained `DofMap` needs and what the penalty and mortar
families do instead.

!listing test/tests/problems/eigen_problem/constraints/nodal_rows_eigen.i block=Constraints

### Which nodes receive a row

A secondary node is left unconstrained in three cases, each of which would otherwise produce a row
that is not a constraint or that contradicts another one:

- The node is one of the primary nodes. It already holds the value the tie would give it, and a row
  that constrains a degree of freedom to itself is not a constraint.
- An enabled [NodalBC](syntax/BCs/index.md) on the secondary variable pins the node. libMesh applies
  a constraint row after the nodal boundary conditions, so the row would override the boundary
  condition; leaving the node alone lets the boundary condition win, which is what `penalty` and
  `kinematic` do.
- The node is not present on this rank.

Rows are added with `forbid_constraint_overwrite = true`, so a secondary degree of freedom that
libMesh already constrains, a hanging node of an adapted mesh for instance, raises an error instead
of silently losing one of the two relations.

When the secondary side is given as a boundary rather than as a list of node ids, the boundary is
read at the moment the rows are built, not at construction. libMesh rebuilds the rows inside
`EquationSystems::reinit()`, before MOOSE delivers the mesh-changed notification, so a node that
refinement adds to that boundary is tied in the same rebuild that created it.

### Parallel and displaced mesh

`addConstraintRows()` is collective: libMesh calls it on every rank each time it rebuilds the
constraints of the system, and each rank adds the rows of the secondary nodes it holds. A rank that
holds a secondary node needs every primary node of the tie, so a tie whose primary and secondary
nodes are far apart needs a replicated mesh. A rank that cannot build a row records the reason, the
ranks exchange their messages, and they all raise the same error, so the run stops rather than
hanging.

The nodes and the degrees of freedom of a row are resolved on the reference mesh and in the
reference nonlinear system whatever `use_displaced_mesh` says, because libMesh builds the
constraints of the reference system before the displaced copy has any degrees of freedom. The two
systems share their numbering, so both receive the same rows. This is why `use_displaced_mesh =
true` stays legal for a nodal tie under `rows`, unlike [MultiPointConstraint.md]: the coefficients
of this tie are input parameters, not geometry.

### Changing the weights between solves

`weights` on [LinearNodalConstraint.md] is controllable. Once per solve, and only for objects that
use `rows`, the framework asks every active row provider whether its rows have changed, agrees on
the answer across the ranks, and rebuilds the rows of the system when any provider reports a change.
A [Controls](syntax/Controls/index.md) object that changes `weights` at the beginning of a time step
therefore changes the tie that step solves. Under `penalty` and `kinematic` the object keeps the
weights it read at construction.

The rebuild covers the coefficients of an existing set of rows and not the pattern of the tie: the
sparsity pattern and the send list are built once. Changing which degrees of freedom are tied is a
mesh change, not a controlled parameter.

!listing test/tests/constraints/nodal_constraint/rows_reinit.i block=Constraints
